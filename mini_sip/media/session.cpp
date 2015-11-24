#include "session.h"

#include <signal.h>

#include "call_recorder.h"
#include "media_stream.h"
#include "media.h"
#include "audio_media.h"

#ifdef VIDEO_SUPPORT
#include "video_media.h"
#include "video_codec.h"
#endif

#include "dtmf_sender.h"
#include "codec.h"
#include "sdp_packet.h"

#include "sdp_headerv.h"
#include "sdp_headert.h"
#include "sdp_headerc.h"
#include "sdp_headera.h"
#include "sdp_headerm.h"
#include "sdp_headers.h"
#include "sdp_headero.h"
#include "sdp_headerb.h"

#include "sip_dialog_config.h"

#include "key_agreement.h"
#include "key_agreement_dh.h"

#include "dbg.h"
#include "string_utils.h"
#include "timestamp.h"
#include "my_time.h"

#include "mini_sip.h"

#include "mikey.h"

#ifdef LOGGING_SUPPORT
#include "logging_manager.h"
#endif

#ifdef ZRTP_SUPPORT
#include "zrtp_host_bridge_minisip.h"
#endif

#ifdef _WIN32_WCE
#	include "minisip_wce_extra_includes.h"
#endif

#define SESSION_LINE "s=Minisip Session"

// pn501 Added for multicodec list operations
using namespace std;

class Mikey_Config : public IMikey_Config
{
public:
    Mikey_Config( SRef<Sip_Identity*> aIdentity ):
        identity(aIdentity) {}

    const std::string get_uri() const
    {
        return identity->get_sip_uri().get_protocol_id() + ":" +
                identity->get_sip_uri().get_user_ip_string();
    }

    SRef<Sip_Sim*> get_sim() const
    {
        return identity->get_sim();
    }
    SRef<Certificate_Chain*> get_peer_certificate() const
    {
        return NULL;
    }
    size_t get_psk_length() const
    {
        return identity->get_psk().size();
    }

    const byte_t* get_psk() const{
        return (byte_t*)identity->get_psk().c_str();
    }

    bool is_method_enabled( int kaType ) const
    {
        switch( kaType )
        {
        case KEY_AGREEMENT_TYPE_DH:
        case KEY_AGREEMENT_TYPE_PK:
        case KEY_AGREEMENT_TYPE_RSA_R:
            return identity->dh_enabled;
        case KEY_AGREEMENT_TYPE_PSK:
        case KEY_AGREEMENT_TYPE_DHHMAC:
            return identity->psk_enabled;
        default:
            return false;
        }
    }

    bool is_cert_check_enabled() const
    {
        return identity->check_cert;
    }
private:
    SRef<Sip_Identity*> identity;
};

Session_Registry * Session::registry = NULL;
SRef<Key_Agreement *> Session::precompute_ka = NULL;

Session::Session(Mini_Sip* _minisip, std::string _callId, std::string localIp, SRef<Sip_Identity*> ident, IStreams_Player_Report_Timestamps *_streams_playerr, std::string localIp6 )
    : minisip(_minisip),
      pid(0),
      started(false),
      local_ip_string(localIp),
      local_ip6_string(localIp6),
      call_id(_callId),
      streams_playerr(_streams_playerr),
      keyframeRequestCallback(NULL),
      timestampOfLastKeyframeRequest_ms(0),
      keyframeRequestThreadQuit(false),
      keyframeRequestCalled(false)
{
    identity = ident;
    ka_type = ident->ka_type;

    dtmf_to_provider = new Timeout_Provider<Dtmf_Event *, SRef<Dtmf_Sender *> >;
    Session::precompute_ka = NULL;

    muted_senders = true;
    silenced_sources = false;
    profile_saved = 1;
    SdpVersionNumber = 3344;

    if( registry )
        registry->register_session( this );

    keyframeRequestThread = new Thread(this, Thread::High_Priority);
}

int Session::get_profile()
{
    return this->profile_saved;
}

void Session::unregister()
{
    if( registry ){
        registry->unregister_session( this );
    }

    if( Session::precompute_ka.is_null() && identity && identity->get_sim() )
    {
        Key_Agreement_DH* ka = NULL;
        ka = new Key_Agreement_DH( identity->get_sim() );
        ka->set_group( DH_GROUP_OAKLEY5 );
        Session::precompute_ka = ka;
    }
}

Session::~Session()
{
    if ( dtmf_to_provider )
        dtmf_to_provider->stop_thread();
    dtmf_to_provider = NULL;
    keyframeRequestThreadQuit = true;

    if(keyframeRequestThread)
    {
        keyframeRequestMutex.lock();
        keyframeRequestCalled = true;
        keyframeRequestCondVar.broadcast();
        keyframeRequestMutex.unlock();
        keyframeRequestThread->join();
        cout << "Keyframe request thread finished" << endl;
        delete keyframeRequestThread;
    }
}

static string matchAnat(string group, SRef<Sdp_Packet *> offer, string local_ip_string, string local_ip6_string)
{
    std::vector<string> groups;

#ifdef DEBUG_OUTPUT
    cerr << "Found group:" << group << endl;
#endif
    if( group.substr(0, 5) == "ANAT "){
        size_t start = 5;
        for(;;){
            string id;
            size_t pos = group.find(' ', start);

            if( pos == string::npos )
                id = group.substr( start );
            else
                id = group.substr( start, pos - start );
            groups.push_back( id );
#ifdef DEBUG_OUTPUT
            cerr << "Found id: " << id << endl;
#endif
            if( pos == string::npos )
                break;

            start = pos + 1;
        }
    }

    // Search for an ANAT group id with all valid streams.
    vector<string>::iterator j;
    for( j = groups.begin(); j != groups.end(); j++ ){
        string id = *j;
        bool idOk = false;

        unsigned int i;
        for( i = 0; i < offer->get_headers().size(); i++ )
        {
            if( offer->get_headers()[i]->get_type() != SDP_HEADER_TYPE_M )
                continue;

            SRef<Sdp_HeaderM *> offerM = (Sdp_HeaderM*)*offer->get_headers()[i];
            string curId = offerM->get_attribute("mid", 0);
            if( id != curId ){
#ifdef DEBUG_OUTPUT
                cout << "Skip id:" << curId << endl;
#endif
                continue;
            }

#ifdef DEBUG_OUTPUT
            cout << "Header " << i << endl;
#endif
            SRef<Sdp_HeaderC *> c = offerM->get_connection();

            if( !c ){
#ifdef DEBUG_OUTPUT
                cout << "No connection header:" << endl;
#endif
                // Ignore, may be an unsupported media type
                continue;
            }

            if( c->get_net_type() != "IN" )
            {
#ifdef DEBUG_OUTPUT
                cout << "Unsupported net type:" << c->get_net_type() << endl;
#endif
                idOk = false;
                break;
            }

            if( c->get_addr_type() != "IP4" && local_ip6_string.empty() ){
#ifdef DEBUG_OUTPUT
                cout << "Unsupported addr type:" << c->get_addr_type() << endl;
#endif
                idOk = false;
                break;
            }

            if( c->get_addr_type() != "IP6" && local_ip_string.empty() ){
                cout << "Unsupported addr type:" << c->get_addr_type() << endl;
                idOk = false;
                break;
            }

            if( offerM->get_port() == 0 ){
#ifdef DEBUG_OUTPUT
                cout << "Disabled port:" << endl;
#endif
                // Ignore, may be an unsupported media type
                continue;
            }

#ifdef DEBUG_OUTPUT
            cerr << "Found valid group id:" << curId << endl;
#endif
            idOk = true;
        }

        if( idOk ){
#ifdef DEBUG_OUTPUT
            cout << "Return id:" << id << endl;
#endif
            return id;
        }
    }

    return "";
}

SRef<Sdp_Packet *> Session::empty_sdp()
{
    SRef<Sdp_Packet *> result;

    result = new Sdp_Packet;

    SRef<Sdp_Header*> v = new Sdp_HeaderV(0);
    result->add_header(v);

    string ipString;
    string addrtype;

    if( !local_ip_string.empty() )
    {
        ipString = local_ip_string;
        addrtype = "IP4";
    }
    else
    {
        ipString = local_ip6_string;
        addrtype = "IP6";
    }

    SRef<Sdp_Header*> o = new Sdp_HeaderO("","3344","3344","IN",
                                          addrtype, ipString );
    result->add_header(o);

    SRef<Sdp_Header*> s = new Sdp_HeaderS(SESSION_LINE);
    result->add_header(s);

    SRef<Sdp_Header*> b = new Sdp_HeaderB("b=CT:10000"); // dummy 10MBit/s
    result->add_header(b);

    SRef<Sdp_Header*> t = new Sdp_HeaderT(0,0);
    result->add_header(t);
    return result;
}

bool Session::add_realtime_media_to_offer(SRef<Sdp_Packet*> result, const std::string &peerUri, bool anatSupported, std::string transport)
{
    list< SRef<Realtime_Media_Stream_Receiver *> >::iterator i;
    list< SRef<Realtime_Media_Stream_Sender *> >::iterator iSender;
    std::list<std::string>::iterator iAttribute;
    std::list<std::string> attributes;
    string type;
    uint16_t localPort = 0;
    SRef<Sdp_HeaderM *> m;
    std::list<SRef<Codec_Description *> > codecs;
    std::list<SRef<Codec_Description *> >::iterator iC;
    uint8_t payloadType;
    string rtpmap;
    bool anat = false;


    if( anatSupported && !local_ip_string.empty() && !local_ip6_string.empty() )
    {
        anat = true;
        result->set_session_level_attribute( "group", "ANAT 1 2" );
    }


    realtime_media_stream_senders_lock.lock();

    iSender = realtime_media_stream_senders.begin();

    int charis = 0;
    for( i = realtime_media_stream_receivers.begin(); i != realtime_media_stream_receivers.end(); i++, iSender++ )
    {
        charis++;
        /* if in the list we have a Realtime_Media_Stream_Receiver_Participant we have to create
         one media line in this SDP packet for this receiver that represents the aggreed SDP medias
         between the newly added participant that this media line represents and Bob the other side of the call
         We do so, so as to avoid Re-Invites to the newly added participant. We bas on the fact that Bob is going
        to support the media characterestics on that line , since bob is the one that aggreed on those when adding the participant.

        The ideal it would be to send back a Re-Invite to the newly added participant and get its offer.....
        This can take place only when the participant that added the new one creates an empty  Re-Invite to Bob most possible for adding another
        participant.
        */
        SRef <Realtime_Media_Stream_Receiver * > streamerTmp = *i;
        SRef < Realtime_Media_Stream_Receiver_Participant * > receiverParticipant = dynamic_cast < Realtime_Media_Stream_Receiver_Participant *> (*streamerTmp);
        if (receiverParticipant)
        {
            result->add_header(*(receiverParticipant->get_media_header()));
            //3-party
            receiverParticipant->set_position_media_line(charis);
            SRef < Realtime_Media_Stream_Sender_Participant * > senderParticipant = dynamic_cast < Realtime_Media_Stream_Sender_Participant *> (*(*iSender));
            senderParticipant->set_position_media_line(charis);
            continue;
        }

        //3-party
        (*i)->set_position_media_line(charis);
        (*iSender)->set_position_media_line(charis);
        codecs = (*i)->get_available_codecs();

        type = (*i)->get_sdp_media_type();
        m = new Sdp_HeaderM( type, localPort, 1, transport );

        if ( (*i)->_disabled == true && (*i)->is_initiated() == true  )
        {
            m->set_port(0);
            result->add_header( *m );
            continue;
        }


        for( iC = codecs.begin(); iC != codecs.end(); iC ++ )
        {
            payloadType = (*iC)->get_sdp_media_type();
            rtpmap = (*iC)->get_sdp_media_attributes();

            m->add_format( itoa(payloadType) );
            if( rtpmap != "" )
            {
                SRef<Sdp_HeaderA*> a = new Sdp_HeaderA("a=X");
                a->set_attributes( "rtpmap:" + itoa( payloadType) + " " + rtpmap );
                m->add_attribute( *a );
            }
            if( (*iC)->get_codec_name() == "iLBC" ) { //for now, iLBC codec only supports 20ms frames
                SRef<Sdp_HeaderA*> ilbc_fmtp = new Sdp_HeaderA("a=X");
                ilbc_fmtp->set_attributes("fmtp:" + itoa( payloadType) + " mode=20" );
                m->add_attribute(*ilbc_fmtp);
            }
            if( (*iC)->get_codec_name() == "H.264" ) {
                SRef<Sdp_HeaderA*> h264_fmtp = new Sdp_HeaderA("a=X");
                //h264_fmtp->set_attributes("fmtp:" + itoa( payloadType) + " profile-level-id=42900b" );
                h264_fmtp->set_attributes("fmtp:" + itoa( payloadType) + " profile-level-id=4d4033" );
                m->add_attribute(*h264_fmtp);
            }

        }
        if (type=="audio")
        {
            //added static DTMF SDP headers in INVITE to audio media
            m->add_format( "101" );
            SRef<Sdp_HeaderA*> dtmf = new Sdp_HeaderA("a=X");
            dtmf->set_attributes("rtpmap:101 telephone-event/8000");
            m->add_attribute(*dtmf);
            SRef<Sdp_HeaderA*> dtmf_fmtp = new Sdp_HeaderA("a=X");
            dtmf_fmtp->set_attributes("fmtp:101 0-15");
            m->add_attribute(*dtmf_fmtp);
            m->set_bandwidth(new Sdp_HeaderB("b=TIAS:128000")); // dummy 128 kbit/s for audio
        }
        else
            m->set_bandwidth(new Sdp_HeaderB("b=TIAS:10000000")); // dummy 10 Mbit/s for video

        result->add_header( *m );

        attributes = (*i)->get_sdp_attributes();
        for( iAttribute = attributes.begin(); iAttribute != attributes.end(); iAttribute ++ )
        {
            SRef<Sdp_HeaderA*> a = new Sdp_HeaderA("a=X");
            a->set_attributes( *iAttribute );
            m->add_attribute( *a );
        }
        /******************* add sendrecv,sendonly..... attribute */
        SRef<Sdp_HeaderA*> a = new Sdp_HeaderA("a=X");
        a->set_attributes( (*i)->_direction_attribute );
        m->add_attribute( *a );

        if( anat )
        {
            SRef<Sdp_HeaderM*> m4 = m;
            SRef<Sdp_HeaderM*> m6 = new Sdp_HeaderM( **m );

            // IPv4
            m4->set_port( (*i)->get_port("IP4") );

            SRef<Sdp_HeaderA*> mid2 = new Sdp_HeaderA( "a=mid:2" );

            m4->add_attribute( mid2 );

            SRef<Sdp_HeaderC*> conn4 = new Sdp_HeaderC( "IN", "IP4", local_ip_string );
            conn4->set_priority( m4->get_priority() );
            m4->set_connection( *conn4 );

            //RFC3605 rtp port signalling using STUN to signal external rtcp port number.
            SRef<Sdp_HeaderA*> rtcpattr4 = new Sdp_HeaderA("a=X");
            rtcpattr4->set_attributes("rtcp:"+itoa( (*i)->get_rtcp_port( "IP4" ) ));
            m4->add_attribute(rtcpattr4);


            // IPv6
            // I have to create a  media for that media line also !!! and place just after that receiver and sender !!!
            m6->set_port( (*i)->get_port("IP6") );

            SRef<Sdp_HeaderA*> mid1 = new Sdp_HeaderA( "a=mid:1" );
            m6->add_attribute( mid1 );


            SRef<Sdp_HeaderC*> conn6 = new Sdp_HeaderC( "IN", "IP6", local_ip6_string );
            conn6->set_priority( m6->get_priority() );
            m6->set_connection( *conn6 );

            //RFC3605 rtp port signalling using STUN to signal external rtcp port number.
            SRef<Sdp_HeaderA*> rtcpattr6 = new Sdp_HeaderA("a=X");
            rtcpattr6->set_attributes("rtcp:"+itoa( (*i)->get_rtcp_port( "IP6" ) ));
            m6->add_attribute(rtcpattr6);

            result->add_header( *m6 );
        }
        else{
            string ipString;
            string addrtype;

            if( !local_ip_string.empty() )
            {
                ipString = local_ip_string;
                addrtype = "IP4";
            }
            else{
                ipString = local_ip6_string;
                addrtype = "IP6";
            }

            SRef<Sdp_HeaderC*> c = new Sdp_HeaderC("IN", addrtype, ipString );
            m->set_connection(c);
            m->set_port( (*i)->get_port( addrtype ) );

            //RFC3605 rtp port signalling using STUN to signal external rtcp port number.
            SRef<Sdp_HeaderA*> rtcpattr = new Sdp_HeaderA("a=X");
            rtcpattr->set_attributes("rtcp:"+itoa( (*i)->get_rtcp_port( addrtype ) ));
            m->add_attribute(rtcpattr);
        }
        //cerr<<"::======================> getOffer adding participant at line which is not participant or stoped  " << charis <<"  \n" ;
    }
    realtime_media_stream_senders_lock.unlock();
#ifdef DEBUG_OUTPUT
    //	cerr << "Session::get_sdp_offer: " << endl << result->get_string() << endl << endl;
#endif

    return true;
}

bool Session::add_reliable_media_to_offer(SRef<Sdp_Packet*> result, const std::string &peer_uri, bool anatSupported)
{
    list< SRef<Reliable_Media_Stream*> >::iterator i;
    std::list<std::string>::iterator iAttribute;
    std::list<std::string> attributes;

    string type;
    uint16_t localPort = 0;
    SRef<Sdp_HeaderM *> m;
    std::list<SRef<Codec_Description *> > codecs;
    std::list<SRef<Codec_Description *> >::iterator iC;
    string rtpmap;

    for( i = reliable_media_sessions.begin(); i != reliable_media_sessions.end(); i++ )
    {
        my_assert(*i);
        type = (*i)->get_sdp_media_type();
        m = new Sdp_HeaderM( type, localPort, 1, (*i)->get_transport() );

        m->add_format( (*i)->get_media_formats() );
        result->add_header( *m );

        attributes = (*i)->get_sdp_attributes();
        for( iAttribute = attributes.begin(); iAttribute != attributes.end(); iAttribute ++ )
        {
            SRef<Sdp_HeaderA*> a = new Sdp_HeaderA("a=X");
            a->set_attributes( *iAttribute );
            m->add_attribute( *a );
        }

#if ADD_ANAT_HEADER_TO_RELIABLE_MEDIA_FIXME
        if( anat )
        {
            SRef<Sdp_HeaderM*> m4 = m;
            SRef<Sdp_HeaderM*> m6 = new Sdp_HeaderM( **m );
            // IPv4
            m4->set_port( (*i)->get_port("IP4") );

            SRef<Sdp_HeaderA*> mid2 = new Sdp_HeaderA( "a=mid:2" );

            m4->add_attribute( mid2 );

            SRef<Sdp_HeaderC*> conn4 = new Sdp_HeaderC( "IN", "IP4", local_ip_string );
            conn4->set_priority( m4->get_priority() );
            m4->set_connection( *conn4 );

            // IPv6
            m6->set_port( (*i)->get_port("IP6") );

            SRef<Sdp_HeaderA*> mid1 = new Sdp_HeaderA( "a=mid:1" );
            m6->add_attribute( mid1 );


            SRef<Sdp_HeaderC*> conn6 = new Sdp_HeaderC( "IN", "IP6", local_ip6_string );
            conn6->set_priority( m6->get_priority() );
            m6->set_connection( *conn6 );

            result->add_header( *m6 );
        }
        else
#endif
        {
            string ipString;
            string addrtype;

            if( !local_ip_string.empty() )
            {
                ipString = local_ip_string;
                addrtype = "IP4";
            }
            else{
                ipString = local_ip6_string;
                addrtype = "IP6";
            }

            SRef<Sdp_HeaderC*> c = new Sdp_HeaderC("IN", addrtype, ipString );
            m->set_connection(c);

            m->set_port( (*i)->get_port( addrtype ) );
        }

    }

#ifdef DEBUG_OUTPUT
    //	cerr << "Reliable Session::getSdpOffer: " << endl << result->get_string() << endl << endl;
#endif

    return true;
}


SRef<Sdp_Packet *> Session::get_sdp_offer( const std::string &peerUri , bool anatSupported )
{
    string keyMgmtMessage;
    const char *transport = NULL;
    SRef<Sdp_Packet *> result;

    result = empty_sdp();

    if( identity->security_enabled )
    {
        int type = 0;
        // FIXME
        switch( ka_type )
        {
        case KEY_MGMT_METHOD_MIKEY_DH:
            type = KEY_AGREEMENT_TYPE_DH;
            break;
        case KEY_MGMT_METHOD_MIKEY_PSK:
            type = KEY_AGREEMENT_TYPE_PSK;
            break;
        case KEY_MGMT_METHOD_MIKEY_PK:
            type = KEY_AGREEMENT_TYPE_PK;
            break;
        case KEY_MGMT_METHOD_MIKEY_DHHMAC:
            type = KEY_AGREEMENT_TYPE_DHHMAC;
            break;
        case KEY_MGMT_METHOD_MIKEY_RSA_R:
            type = KEY_AGREEMENT_TYPE_RSA_R;
            break;
        default:
            mikey = NULL;
            return NULL;
        }

        SRef<Sdp_HeaderA *> a;
        Mikey_Config *config = new Mikey_Config( identity );
        // FIXME free config
        mikey = new Mikey( config );

        add_streams();

        keyMgmtMessage = mikey->initiator_create( type, peerUri );
        if( mikey->error() ){
            // something went wrong
            return NULL;
        }
        result->set_session_level_attribute( "key-mgmt", keyMgmtMessage );
        transport = "RTP/SAVP";
    }
    else{
        transport = "RTP/AVP";
    }

    if (!add_realtime_media_to_offer(result, peerUri, anatSupported, transport))
    {
        return NULL;
    }
    if (!add_reliable_media_to_offer(result, peerUri, anatSupported) )
    {
        return NULL;
    }

    set_last_sdp_packet(result);
    return result;
}

bool Session::set_sdp_answer( SRef<Sdp_Packet *> answer, std::string peerUri )
{
    unsigned int i;
    int j;
    SRef<Realtime_Media_Stream_Receiver *> receiver;
    bool found = false;

    this->peer_uri = peerUri;
#ifdef DEBUG_OUTPUT
    // 	cerr << "Session::setSdpAnswer" << endl;
#endif
    if( mikey ){
        /* get the keymgt: attribute */
        string keyMgmtMessage =
                answer->get_session_level_attribute( "key-mgmt" );

        if( !peerUri.empty() ){
            mikey->get_key_agreement()->set_peer_uri( peerUri );
        }

        if( !mikey->initiator_authenticate( keyMgmtMessage ) )
        {
            error_string = "Could not authenticate the key management message";
            fprintf( stderr, "Auth failed\n");
            return false;
        }
        this->peer_uri = mikey->peer_uri();

        string mikeyErrorMsg = mikey->initiator_parse();
        if( mikeyErrorMsg != "" )
        {
            error_string = "Could not parse the key management message. ";
            error_string += mikeyErrorMsg;
            fprintf( stderr, "Parse failed\n");
            return false;
        }

    }

    string group = answer->get_session_level_attribute("group");
    string selectedId;

    if( !group.empty() )
        selectedId = matchAnat(group, answer, local_ip_string, local_ip6_string);
#ifdef DEBUG_OUTPUT
    cout << "Selected:" << selectedId << endl;
#endif

    SRef<Sdp_HeaderC*> sessionConn = answer->get_session_level_connection();
    int indexMediaLine = 0 ;
    /*getLastSdpAnswerMcu is  only used when in call with the mcu */
    set_last_sdp_answer_mcu(answer);
    for( i = 0; i < answer->get_headers().size(); i++ )
    {
        if( answer->get_headers()[i]->get_type() == SDP_HEADER_TYPE_M ){

            indexMediaLine++;

            SRef<Sdp_HeaderM *> m = ((Sdp_HeaderM*)*(answer->get_headers()[i]));
#ifdef DEBUG_OUTPUT
            cerr << "Session::setSdpAnswer - trying media line " << m->get_string() << endl;
#endif

            if( !group.empty() )
            {
#ifdef DEBUG_OUTPUT
                cout << "Media " << i << endl;
#endif
                const string &id = m->get_attribute("mid", 0);
#ifdef DEBUG_OUTPUT
                cout << "id: " << id << endl;
#endif
                if( id != selectedId ){
#ifdef DEBUG_OUTPUT
                    cerr << "Skip unselected id:" << id << endl;
#endif
                    // I should stop the appropriate sender and receiver !!!
                    continue;
                }
            }

            SRef<Sdp_HeaderC *> c = m->get_connection();
            SRef<IPAddress *> remoteAddress;
            if( !c )
                c = sessionConn;

            if( !c )
            {
                cerr << "Session::setSdpAnswer - skip missing connection" << endl;
                continue;
            }

            remoteAddress = c->get_ipadress();

            for( j = 0; j < m->get_nr_formats(); j++ ){
                receiver = match_format( m, j, remoteAddress, indexMediaLine );
                /*
                  having received the answer to one Invite if one media line is for
                  the newly added agent, then we are going to take no actions for that media line We consider that the other participant
                  has sent the same offer since we program it do it so , so as to avoid Re-Invites. However, the ideal would be to
                  match the media lines and if they are not the same to send a bye to the new added agent
                */
                SRef < Realtime_Media_Stream_Receiver_Participant * > SRParticipant;
                if (receiver)
                    SRParticipant = dynamic_cast < Realtime_Media_Stream_Receiver_Participant *> (*receiver);
                if (SRParticipant)
                    break;

                if( !receiver )
                    continue;
#ifdef DEBUG_OUTPUT
                cerr << "Session::setSdpAnswer - Found receiver at " << remoteAddress->get_string() << endl;
                cerr << "Receiver found: " << !!receiver << endl;
#endif

                if( receiver && m->get_port() == 0 )
                {
                    /* This offer was rejected */
                    receiver->_disabled = true;

                    // FIXME close socket
                    break ; // we exit from the loop of mutching media formats for that media line since  Minisip was asked to terminate that line
                }
                else{

                    /* Be ready to receive */
                    SRef < Sdp_HeaderM * > tmp;
                    tmp = set_direction_attribute( m , NULL ,  receiver,  indexMediaLine );
                    receiver->set_media_header(m);
                    receiver->start();
                    found = true;
                    break;// !!!!!!!
                }
            }
            if ( found == false ){
                // we should stop the receiver that was on that line and set the disabled == true
            }
        }
    }
    return found;
}

SRef<Realtime_Media_Stream_Receiver *> Session::match_format( SRef<Sdp_HeaderM *> m,
                                                              uint32_t iFormat, SRef<IPAddress *> &remoteAddress, int index )
{
    if(remoteAddress.is_null())
        return NULL;
    list<SRef<Realtime_Media_Stream_Sender *> >::iterator iSStream;
    list<SRef<Realtime_Media_Stream_Receiver *> >::iterator iRStream;
    SRef < Rtp_Receiver *> rtpReceiver;
    SRef < Rtp_Receiver *> rtp6Receiver;
    /* If we have a sender for this format, activate it */
#ifdef DEBUG_OUTPUT
    my_dbg("media") << "Session::match_format: Starting senders loop" << endl;
#endif

    uint8_t j = 1;
    bool addition = true;
    realtime_media_stream_senders_lock.lock();
    for (iSStream = realtime_media_stream_senders.begin(); iSStream != realtime_media_stream_senders.end(); iSStream++, j++)
    {

#ifdef DEBUG_OUTPUT
        my_dbg("media") << "Trying a sender"<< endl;
#endif
        /* we enter in that IF , only when we have already created one RealTimeStreamMedia for that media line or if the already two streamers that have created have not being uesed yet */
        if(index == (*iSStream)->get_position_media_line()  || (*iSStream)->get_position_media_line() == 0 ){
            //cerr<<"========================> Found sender at that line but it addition ???\n";
            if ( (*iSStream)->get_sdp_media_type().compare(m->get_media()) == 0 ) {addition = false; }
            else{continue;}
            if ( m->get_port()== 0)
            {
                // check if this streamsender has already been stopped
                // if not termnate it and continue otherwise just continue to the others senders
                (*iSStream)->set_port(0);
                (*iSStream)->_disabled = true;
                (*iSStream)->sendonly = false;
                (*iSStream)->stop();
                break;
            }
            /* if the Rtp_Streamer is one created for the one participant, of Class StreamerParticipant added by this Voip object then do nothing break*/
            SRef <Realtime_Media_Stream_Sender_Participant *> SSParticipant = dynamic_cast< Realtime_Media_Stream_Sender_Participant * > (*(*iSStream));
            if ( SSParticipant )
            {
                // normall behaivor we should generate a Re-Invite however only refresh port and address for the forwarder
                SSParticipant->set_port((uint16_t) m->get_port());
                SSParticipant->set_remote_address(remoteAddress);
                break;
            }
            if ((*iSStream)->matches(m, iFormat)) {
#ifdef DEBUG_OUTPUT
                my_dbg("media") << "Found sender for " << (*iSStream)->get_sdp_media_type()<< endl;
#endif

                (*iSStream )->set_position_media_line(index);

#if 0
                if( ka ) {
                    ka->add_srtp_stream( (*iStream)->get_ssrc(),
                                         0, /*ROC */
                                         0, /* policy (fix me) */
                                         2*j/* CSID */
                                         );
                }
#endif
#ifdef DEBUG_OUTPUT
                cerr << "Set remote: " << remoteAddress->get_string() << "," << m->get_port() << endl;
#endif
                // we have found media format matching for this sender and we set
                // disabled false. There is a chance that with a previous negotiation that this media stream was stoped
                (*iSStream)->_disabled = false;
                if ((*iSStream)->get_sdp_media_type().compare("video") == 0) {
                    this->set_destination_port((uint16_t) m->get_port());
                    this->set_destination_ip(remoteAddress);
                    //					string fmtpParam = m->get_fmtp_param(m->get_format(iFormat));
                    //					size_t pos = fmtpParam.find('=');

                    //					cerr<<"TTA fmtpparam = "<<fmtpParam<<" substring = "<< fmtpParam.substr(pos + 1, 2)<<endl;
                    //					if (fmtpParam.substr(pos + 1, 2).compare("42") == 0){
                    //						cerr<<"TTA: Setting Baseline profile"<<endl;
                    //						this->profile_saved = 0;
                    //						this->sendingWidth = 1920;
                    //						this->sendingHeight = 1080;
                    //					}else{
                    //						this->profile_saved = 1;
                    //						this->sendingWidth = 1920;
                    //						this->sendingHeight = 1080;
                    //					}
                }
                (*iSStream)->set_port((uint16_t) m->get_port());
                (*iSStream)->set_remote_address(remoteAddress);
                break; // we matched the sender no need to iterate to the list of senders
            }// matches end
            break;// we found the sender no need to iterate
        }// end if line match
    }// iteration of senders endedW
    realtime_media_stream_senders_lock.unlock();
    if ( addition )
    {
        /* we have to create an additional streamSenderfor that media line  and then call the matches function */
        bool foundMatch = false;
        //  cerr<<" ====================> match_format calls for additional rtp senders and receivers " <<  index <<" \n";
        SRef<Realtime_Media_Stream_Sender *> sStream = create_real_time_media_streams ( index , m->get_media(),"a=sendrecv" );
        int Jformat;
        for(Jformat = 0; Jformat < m->get_nr_formats() && sStream ; Jformat++ )
        {
            if (sStream->matches(m, Jformat))
            {
                foundMatch = true;
                sStream->_disabled = false;
#ifdef DEBUG_OUTPUT
                my_dbg("media") << "Found sender for " << sStream->get_sdp_media_type()<< endl;
#endif
#if 0
                if( ka ) {
                    ka->add_srtp_stream( sStream->get_ssrc(),
                                         0, /*ROC */
                                         0, /* policy (fix me) */
                                         2*j/* CSID */
                                         );
                }
#endif
#ifdef DEBUG_OUTPUT
                cerr << "Set remote: " << remoteAddress->get_string() << "," << m->get_port() << endl;
#endif

                //      				if (sStream->get_sdp_media_type().compare("video") == 0) {
                //								string fmtpParam = m->get_fmtp_param(m->get_format(iFormat));
                //								size_t pos = fmtpParam.find('=');
                //                cerr<<"TTA fmtpparam = "<<fmtpParam<<" substring = "<< fmtpParam.substr(pos + 1, 2)<<endl;
                //	              if(fmtpParam.substr(pos + 1, 2).compare("42") == 0){
                //									cerr<<"TTA: Setting Baseline profile"<<endl;
                //                  this->profile_saved = 0;
                //                  this->sendingWidth = 1920;
                //                  this->sendingHeight = 1080;
                //                }
                //								else{
                //									this->profile_saved = 1;
                //									this->sendingWidth = 1920;
                //                  this->sendingHeight = 1080;
                //                }
                //							}
                sStream->set_port((uint16_t) m->get_port());
                sStream->set_remote_address(remoteAddress);
            }// matches finished
        }//iteration of formats finished
        if ( !foundMatch && sStream )
        {
            sStream->_disabled = true;// there is no matching for the new sender thus this sender should not start
            sStream->set_port(0);
            sStream->sendonly = false;
            sStream->stop();
        }
    }// if addition finished
    addition = true;
    /* Look for a receiver */
#ifdef DEBUG_OUTPUT
    my_dbg("media") << "Starting receivers loop"<< endl;
#endif

    for (iRStream = realtime_media_stream_receivers.begin(); iRStream	!= realtime_media_stream_receivers.end(); iRStream++)
    {
        if ( index == (*iRStream)->get_position_media_line() || (*iRStream)->get_position_media_line()== 0  )
        {
            if ( (*iRStream)->get_sdp_media_type().compare(m->get_media()) == 0 ) addition = false;
            else continue;
            if ( m->get_port()== 0)
            {
                // check if this streamsender has already been stopped
                // if not termnate it and continue otherwise just continue to the others senders
                (*iRStream)->_disabled = true;
                (*iRStream)->recvonly = false;
                (*iRStream)->stop();
                return (*iRStream);
            }

            /* if the Rtp_Streamer is one created for the one participant, of Class StreamerParticipant added by this Voip object then do nothing break*/
            SRef <Realtime_Media_Stream_Receiver_Participant *> SRP = dynamic_cast< Realtime_Media_Stream_Receiver_Participant * > (*(*iRStream));
            if ( SRP )
            {
                return(*iRStream);
            }

            if ((*iRStream)->matches(m, iFormat))
            {
#ifdef DEBUG_OUTPUT
                my_dbg("media") << "Found receiver for " << (*iRStream)->get_sdp_media_type()<< endl;
#endif
                (*iRStream )->set_position_media_line(index);
                (*iRStream )->_disabled = false;
                return (*iRStream);
            }
        }
    }
    return NULL;
}

bool Session::set_sdp_offer( SRef<Sdp_Packet *> offer, std::string peerUri )
{
    unsigned int i;
    int j;
    SRef<Realtime_Media_Stream_Receiver *> receiver;
    SRef<Sdp_Packet *> packet;
    string keyMgmtMessage;
    std::list<std::string>::iterator iAttribute;
    std::list<std::string> attributes;
    std::list<SRef<Codec_Description *> > codecs;
    std::list<SRef<Codec_Description *> >::iterator iC;
    // 	uint8_t payloadType;
    string rtpmap;
    bool found = false;

    this->peer_uri = peerUri;
    // 	cerr << "Session::setSdpOffer" << endl;

    keyMgmtMessage = offer->get_session_level_attribute( "key-mgmt" );

    if( keyMgmtMessage != "" )
    {
        Mikey_Config *config = new Mikey_Config( identity );
        // FIXME free config
        mikey = new Mikey( config );

        add_streams(); //TODO: This only adds SRTP streams, no reliable media is handled.

        if( !mikey->responder_authenticate( keyMgmtMessage, peerUri ) )
        {
            error_string =  "Incoming key management message could not be authenticated";
            // 			if( ka ){
            error_string += mikey->auth_error();
            // 			}
            return false;
        }
        this->peer_uri = mikey->peer_uri();
        //Here we set the offer in ka
        mikey->set_mikey_offer();
    }
    else{
        /*securityConfig.*/ka_type = KEY_MGMT_METHOD_NULL;
    }


    sdp_answer = empty_sdp();

    string group = offer->get_session_level_attribute("group");
    string selectedId;

    if( !group.empty() ){
        sdp_answer->set_session_level_attribute("group", group);

        selectedId = matchAnat(group, offer, local_ip_string, local_ip6_string);
    }

#ifdef DEBUG_OUTPUT
    cout << "Get remote addr " << endl;
#endif
    SRef<Sdp_HeaderC*> sessionConn = offer->get_session_level_connection();

#ifdef DEBUG_OUTPUT
    cout << "Built empty sdp" << endl;
#endif


    int index_media_line = 0;
    for( i = 0; i < offer->get_headers().size(); i++ ){
#ifdef DEBUG_OUTPUT
        cout << "Header " << i << endl;
#endif

        if( offer->get_headers()[i]->get_type() == SDP_HEADER_TYPE_M )
        {
            index_media_line ++;

            SRef<Sdp_HeaderM *> offerM = (Sdp_HeaderM*)*(offer->get_headers()[i]);

            SRef<Sdp_HeaderM *> answerM = new Sdp_HeaderM(
                        offerM->get_media(), 0, 0,
                        offerM->get_transport() );

            sdp_answer->add_header( *answerM );

            if( !group.empty() ){
#ifdef DEBUG_OUTPUT
                cout << "Media " << i << endl;
#endif
                const string &id = offerM->get_attribute("mid", 0);
                if(!id.empty())
                    answerM->add_attribute(new Sdp_HeaderA("a=mid:" + id));

#ifdef DEBUG_OUTPUT
                cout << "id: " << id << endl;
#endif
                if( id != selectedId )
                {
                    // receiver = match_format( offerM, 0, remoteAddress,index_media_line , true);
                    // I should stop the appropriate sender and receiver !!
                    continue;
                }
            }

            const string &transport = offerM->get_transport();

            if (transport != "RTP/AVP" && !identity->security_enabled && transport == "RTP/SAVP")
            {
                error_string += "No supported SRTP key exchange method";
                return false;
            }

            SRef<Sdp_HeaderC *> c = offerM->get_connection();
            SRef<IPAddress *> remoteAddress;
            string addrString;

            if( !c )
                c = sessionConn;

            if( !c )
                continue;

            if ( c->get_net_type() != "IN" )
                continue;

            if( c->get_addr_type() == "IP4" )
            {
                if( local_ip_string.empty() )
                    continue;
                addrString = local_ip_string;
            }
            else if( c->get_addr_type() == "IP6" )
            {
                if( local_ip6_string.empty() )
                    continue;
                addrString = local_ip6_string;
            }

            remoteAddress = c->get_ipadress();
            if ( !remoteAddress)
            {
                my_err <<"ERROR: could not resolve where to send RTP: <"<<c->get_addr()<<">"<<endl;
                continue;
            }
            my_assert(remoteAddress);
            answerM->set_connection( new Sdp_HeaderC("IN", c->get_addr_type(), addrString ));
            /*
            if the offer has zero por in that media header examined
            I have to stop the stream sender and receiver that has been created for that offer
            and return one media line with zero port value
            */
            if ( offerM->get_port() == 0 )
            {
                /* match format will stop that sender and receiver */
                receiver = match_format( offerM, 0, remoteAddress,index_media_line );
                answerM->set_port (0);

                continue ; // we want to examine the following media lines in the offer....
            }

            for( j = 0; j < offerM->get_nr_formats(); j++ )
            {
                receiver = match_format( offerM, j, remoteAddress,index_media_line );
                /*
                 it is called when the voip agent is forming one SDP answer for one SDP offer.
                 If that agent had added previously one nely added agent, then to the answer that it sends it must be one
                media line for that newly added agent. The agent that forms tha answer must put in that media line the last aggreed
                media line regarding that agent, So as to much with the offer. The offer should include in that media line the characterestics of
                the last agrreed media line. Since the offerer had sent those media lines previously.
                The connection and port field on that answer must be those of the added agent while the media characteristics must be the one
                answered by the agent that we send the media of the added component.

                Ideally we should sent a Re-Invite to the new added agent to answer to the offer.
                */
                if ( ! receiver ) continue;
                SRef < Realtime_Media_Stream_Receiver_Participant * > SRP = dynamic_cast < Realtime_Media_Stream_Receiver_Participant *> (*receiver);
                if (SRP) {
                    sdp_answer->remove_last_header();
                    sdp_answer->add_header(*(SRP->get_media_header()));
                    break;
                }

                if( receiver )
                {
                    if( answerM->get_port() == 0 )
                    {
                        answerM->set_port( receiver->get_port( c->get_addr_type() ) );
                    }
                    else{
                        /* This media has already been treated */
                        continue;
                    }

                    /* found a receiver, accept the offer */
                    //add the payload type to the offer, as accepted ...
                    string payloadTypeAccepted;
                    if(receiver->get_media()->get_sdp_media_type() == "video"){
                        //payloadTypeAccepted = "" + itoa(receiver->get_media()->getDecoderInstance()->get_sdp_media_type());
                        payloadTypeAccepted = "99";//FIXME
                    }
                    else
                        payloadTypeAccepted = offerM->get_format( j );

                    cerr<<"TTA:Payloadtype accepted = "<<payloadTypeAccepted <<endl<<endl;
                    //string payloadStr = itoa( payloadTypeAccepted );
                    answerM->add_format( payloadTypeAccepted );
                    SRef<Sdp_HeaderA*> rtpmap = new Sdp_HeaderA("a=X");
                    SRef<Sdp_HeaderA*> fmtp = new Sdp_HeaderA("a=X");

                    string f = offerM->get_fmtp_param( offerM->get_format(j) );
                    if (f.length()>0){
                        fmtp->set_attributes( "fmtp:" + /*payloadStr*/ payloadTypeAccepted + " " + f );
                        answerM->add_attribute( *fmtp );
                    }

                    f =  offerM->get_rtp_map( offerM->get_format(j));
                    if (f.length()>0){
                        rtpmap->set_attributes(   "rtpmap:" + /*payloadStr*/ payloadTypeAccepted + " " + f );
                        answerM->add_attribute( *rtpmap );
                    }

                    /* Additional attributes (framesize, ...) */

                    attributes = receiver->get_sdp_attributes();
                    for( iAttribute = attributes.begin(); iAttribute != attributes.end(); iAttribute ++ ){
                        SRef<Sdp_HeaderA*> a = new Sdp_HeaderA("a=X");
                        a->set_attributes( *iAttribute );
                        answerM->add_attribute( *a );
                    }

                    /* handling of a = recvonly , sendonly, inactive and sendrecv attributes */
                    answerM = set_direction_attribute( offerM , answerM ,  receiver,  index_media_line );

                    found = true;
                    receiver->set_media_header(answerM);
                    break;
                }else{
                    //cerr << "EEEE: did not find receiver!"<<endl;
                    /*
                        not supported medias ...
                    */
                }
            }
            if ( found == false ){
                // not matching for all the formats we should stop that sender and receiver in case it was running previously
            }

        }
    }
    return found;
}

void Session::add_reliable_media_session(SRef<Reliable_Media_Stream*> rsess)
{
    realtime_media_stream_senders_lock.lock();
    reliable_media_sessions.push_back(rsess);
    realtime_media_stream_senders_lock.unlock();
}

SRef<Sdp_Packet *> Session::get_sdp_answer_200Ok()
{
    set_last_sdp_packet( sdp_answer);
    set_last_sdp_answer( sdp_answer);
    return sdp_answer;
}

SRef<Sdp_Packet *> Session::get_sdp_answer()
{
    if( mikey )
    {
        string keyMgmtAnswer;
        // Generate the key management answer message
        keyMgmtAnswer = mikey->responder_parse();

        if( mikey->error() )
        {
            // Something went wrong
            error_string = "Could not parse key management message.";
            fprintf(stderr, "responder_parse failed\n" );
            return NULL;
        }
        sdp_answer->set_session_level_attribute( "key-mgmt", "mikey "+ keyMgmtAnswer );
    }
    set_last_sdp_packet( sdp_answer);
    set_last_sdp_answer( sdp_answer);
    return sdp_answer;
}

SRef<Sdp_Packet *> Session::get_sdp_answer(SRef < Sdp_Packet * > sdpPack)
{
    if( mikey ){
        add_streams();
        string keyMgmtAnswer;
        // Generate the key management answer message
        keyMgmtAnswer = mikey->responder_parse();

        if( mikey->error() )
        {
            // Something went wrong
            error_string = "Could not parse key management message.";
            fprintf(stderr, "responder_parse failed\n" );
            return NULL;
        }
        sdpPack->set_session_level_attribute( "key-mgmt", "mikey "+ keyMgmtAnswer );
    }
    set_last_sdp_packet( sdpPack);
    set_last_sdp_answer( sdpPack);
    return sdpPack;
}

SRef<Realtime_Media_Stream_Receiver *> Session::get_realtime_media_stream_receiver(uint32_t ssrc)
{
    SRef<Realtime_Media_Stream_Receiver *> ret;
    std::list<SRef<Realtime_Media_Stream_Receiver *> >::iterator i;
    for (i=realtime_media_stream_receivers.begin(); i!=realtime_media_stream_receivers.end(); i++)
        if ( (*i)->get_ssrc_set().count(ssrc)>0 )
            ret = *i;
    return ret;
}

SRef<Realtime_Media_Stream_Sender*> Session::get_realtime_media_stream_sender(uint32_t ssrc)
{
    SRef<Realtime_Media_Stream_Sender*> ret;
    realtime_media_stream_senders_lock.lock();
    std::list<SRef<Realtime_Media_Stream_Sender*> >::iterator i;
    for (i=realtime_media_stream_senders.begin(); i!=realtime_media_stream_senders.end(); i++)
        if ( (*i)->get_ssrc()==ssrc )
            ret = *i;
    realtime_media_stream_senders_lock.unlock();
    return ret;
}

std::list< SRef<Realtime_Media_Stream_Receiver *> > Session::get_realtime_media_stream_receivers()
{
    return realtime_media_stream_receivers;
}

std::list< SRef<Realtime_Media_Stream_Sender *> > Session::get_realtime_media_stream_senders()
{
    return realtime_media_stream_senders;
}

SRef<Sip_Identity*> Session::get_own_identity()
{
    return identity;
}

void Session::start()
{
    if (started)
        return;
    started=true;

    list< SRef<Realtime_Media_Stream_Sender * > >::iterator iS;
    list< SRef<Realtime_Media_Stream_Receiver * > >::iterator iR;

    SRef<Key_Agreement*> ka;

    if( is_secure() )
    {
        ka = mikey->get_key_agreement();
    }

    if( ka ){
#ifdef ENABLE_TS
        ts.save( TGK_START );
#endif
        Key_Agreement_DH_Base *kaDH =
                dynamic_cast<Key_Agreement_DH_Base*>(*ka);
        if( kaDH ){
            kaDH->compute_tgk();
        }
#ifdef ENABLE_TS
        ts.save( TGK_END );
#endif
    }
    for( iR = realtime_media_stream_receivers.begin(); iR != realtime_media_stream_receivers.end(); iR++ )
    {
        if( ! (*iR)->_disabled )
        {
            if ( (*iR)->is_initiated() == false ) (*iR)->set_initiated(true) ;
            if( ka )
            {
                (*iR)->set_key_agreement( ka );
                (*iR)->flush_crypto_contexts();
            }

            (*iR)->start();
        }else{
        }
    }

    realtime_media_stream_senders_lock.lock();
    for( iS = realtime_media_stream_senders.begin(); iS != realtime_media_stream_senders.end(); iS++ )
    {
        if( (*iS)->get_port() )
        {
            if( ka )
            {
                (*iS)->set_key_agreement( ka );
            }
            (*iS)->start();
        }
    }
    realtime_media_stream_senders_lock.unlock();
#ifdef LOGGIN_SUPPORT
    Logger::get_instance()->info(get_call_id(), "info.sessionRegister");
#endif
}

void Session::refresh()
{
    list< SRef<Realtime_Media_Stream_Sender * > >::iterator iS;
    list< SRef<Realtime_Media_Stream_Receiver * > >::iterator iR;

    SRef<Key_Agreement*> ka;


    if( is_secure() )
    {
        ka = mikey->get_key_agreement();
    }

    if( ka )
    {
        Key_Agreement_DH_Base *kaDH =  dynamic_cast<Key_Agreement_DH_Base*>(*ka);
        if( kaDH ){
            kaDH->compute_tgk();
        }
    }


    for( iR = realtime_media_stream_receivers.begin(); iR != realtime_media_stream_receivers.end(); iR++ )
    {
        if ( (*iR)->is_initiated() == false ) (*iR)->set_initiated(true) ;
        if( ! (*iR)->_disabled )
        {
            if( ka ){
                (*iR)->set_key_agreement( ka );
                (*iR)->flush_crypto_contexts();
            }

            (*iR)->start();
        }else{
        }
    }

    realtime_media_stream_senders_lock.lock();
    for( iS = realtime_media_stream_senders.begin(); iS != realtime_media_stream_senders.end(); iS++ )
    {
        if( (*iS)->get_port()!= 0 )
        {
            if( ka ){
                (*iS)->set_key_agreement( ka );
            }
            (*iS)->start();
        }
    }

    realtime_media_stream_senders_lock.unlock();
}

void Session::stop()
{
    if ( get_pid()>0 ) kill(get_pid(),SIGKILL );
    cerr <<"ZZZZ: doing Session::stop"<<endl;
    started=false;
    list< SRef<Realtime_Media_Stream_Sender * > >::iterator iS;
    list< SRef<Realtime_Media_Stream_Receiver * > >::iterator iR;

    cerr <<"ZZZZ: Session::stop stopping all receivers"<<endl;
    for( iR = realtime_media_stream_receivers.begin(); iR != realtime_media_stream_receivers.end(); iR++ )
    {
        if( ! (*iR)->_disabled )
        {
            (*iR)->stop();
        }
    }

    cerr <<"ZZZZ: Session::stop stopping all senders"<<endl;
    realtime_media_stream_senders_lock.lock();
    for( iS = realtime_media_stream_senders.begin(); iS != realtime_media_stream_senders.end(); iS++ )
    {
        if( (*iS)->get_port() )
        {
            (*iS)->stop();
        }
    }
    cerr <<"ZZZZ: Session::stop done stopping all senders"<<endl;
    SRef<Call_Recorder *> cr;
    if (call_recorder)
        cr = dynamic_cast<Call_Recorder *>(*call_recorder);
    if( !cr ) {
#ifdef DEBUG_OUTPUT
        cerr << "Session::stop - no call recorder?" << endl;
#endif
    } else {
        cr->set_allow_start( false );
        cr->free();
    }
    call_recorder = NULL; //stop the call recorder object

    realtime_media_stream_senders_lock.unlock();
#ifdef LOGGING_SUPPORT
    Logger::get_instance()->info(get_call_id(), "info.sessionUnregister");
#endif

    if ( dtmf_to_provider )
        dtmf_to_provider->stop_thread();
    dtmf_to_provider=NULL;
}

void Session::add_realtime_media_stream_receiver( SRef<Realtime_Media_Stream_Receiver *> realtimeMediaStream )
{
    realtime_media_stream_receivers.push_back( *realtimeMediaStream );
    silence_sources( silenced_sources );
}

void Session::add_realtime_media_stream_sender( SRef<Realtime_Media_Stream_Sender *> realtimeMediaStream)
{
    realtime_media_stream_senders_lock.lock();
    realtimeMediaStream->set_muted( muted_senders );
    realtime_media_stream_senders.push_back( *realtimeMediaStream );
    realtime_media_stream_senders_lock.unlock();
}

void Session::remove_realtime_media_stream_sender( SRef<Realtime_Media_Stream_Sender *> realtimeMediaStream )
{
    realtime_media_stream_senders_lock.lock();
    realtime_media_stream_senders.remove( realtimeMediaStream );
    realtime_media_stream_senders_lock.unlock();
}

std::string Session::get_error_string()
{
    return error_string;
}

uint16_t Session::get_error_code()
{
    return error_code;
}

bool Session::is_secure()
{
    return mikey && mikey->is_secured();
}


std::string Session::get_call_id()
{
    return call_id;
}

void Session::set_call_id( const std::string callId )
{
    this->call_id = callId;
#ifdef ZRTP_SUPPORT
    realtime_media_stream_senders_lock.lock();
    for ( std::list< SRef<Realtime_Media_Stream_Sender *> >::iterator it =  realtime_media_stream_senders.begin();
          it !=  realtime_media_stream_senders.end(); it++ ) { // TODO - need better support to set call id in ZHB
        SRef<ZrtpHostBridgeMinisip*> zhb = (*it)->getZrtpHostBridge();
        if (zhb) {
            zhb->setCallId(callId);
        }
    }
    realtime_media_stream_senders_lock.unlock();

#endif
}

void Session::send_dtmf( uint8_t symbol )
{
    SRef<Dtmf_Sender *> dtmfSender = new Dtmf_Sender( this );
    uint32_t * ts = new uint32_t;
    *ts = 0;
    dtmf_to_provider->request_timeout( 0, dtmfSender, new Dtmf_Event( symbol, 10, 0, false, true, ts ) );
    dtmf_to_provider->request_timeout( 5, dtmfSender, new Dtmf_Event( symbol, 10, 0, false, false, ts ) );
    dtmf_to_provider->request_timeout( 10, dtmfSender, new Dtmf_Event( symbol, 10, 0, false, false, ts ) );

    dtmf_to_provider->request_timeout( 15, dtmfSender, new Dtmf_Event( symbol, 10, 800, true, false, ts ) );
    dtmf_to_provider->request_timeout( 20, dtmfSender, new Dtmf_Event( symbol, 10, 800, true, false, ts ) );
    dtmf_to_provider->request_timeout( 25, dtmfSender, new Dtmf_Event( symbol, 10, 800, true, false, ts, true ) );
}

void Session::mute_senders (bool mute)
{
    muted_senders = mute;
    realtime_media_stream_senders_lock.lock();
    for( std::list< SRef<Realtime_Media_Stream_Sender *> >::iterator it =  realtime_media_stream_senders.begin();
         it !=  realtime_media_stream_senders.end(); it++ ) {
        (*it)->set_muted( mute );
    }

    SRef<Call_Recorder *> cr;
    if (call_recorder){
        cr = dynamic_cast<Call_Recorder *>(*call_recorder);
    }

    if( !cr ) {
#ifdef DEBUG_OUTPUT
        cerr << "Session::muteSenders - no call recorder?" << endl;
#endif
    } else {
        cr->set_enabled_mic( !mute );
    }

    realtime_media_stream_senders_lock.unlock();
}

void Session::silence_sources ( bool silence )
{
#ifdef DEBUG_OUTPUT
    /*	if( silence )
        cerr << "Session::SilenceSources - true" << endl;
    else
        cerr << "Session::SilenceSources - false" << endl;
*/
#endif
    silenced_sources = silence;
    for( std::list< SRef<Realtime_Media_Stream_Receiver *> >::iterator it =  realtime_media_stream_receivers.begin();
         it !=  realtime_media_stream_receivers.end(); it++ )
    {
        set<uint32_t>::iterator ssrcIt;
        SRef<Audio_Media *> audioMedia;
        SRef<Audio_Media_Source *> audioSource;

        //obtain the media object used by the media stream, and try to cast it to
        //an audiomedia ...
        audioMedia = dynamic_cast<Audio_Media *>( *( (*it)->get_media() ) );
        //if it is not audiomedia, we are not interested
        if( !audioMedia )
            continue;

        set<uint32_t> &ssrcSet = (*it)->get_ssrc_set();

        for( ssrcIt = ssrcSet.begin(); ssrcIt != ssrcSet.end(); ssrcIt++ ) {
            audioSource = audioMedia->get_source( *ssrcIt );
            if( !audioSource ) {
                // 				cerr << "Session::SilenceSources - skipping ssrc ... no source found" << endl;
                continue;
            } else {
                audioSource->set_silenced( silence );
                // 				cerr << "Session::SilenceSources - silencing source " << itoa(*ssrcIt) << endl;
            }
        }
    }

    if( call_recorder )
    {
        SRef<Call_Recorder *> cr = dynamic_cast<Call_Recorder *>(*call_recorder);
        if( !cr ) {
#ifdef DEBUG_OUTPUT
            cerr << "Session::silenceSources - no call recorder? (1)" << endl;
#endif
        } else {
            cr->set_enabled_network( !silence );
        }
    } else {
#ifdef DEBUG_OUTPUT
        cerr << "Session::silenceSources - no call recorder? (2)" << endl;
#endif
    }
}

std::string Session::get_debug_string()
{
    string ret;
#ifdef DEBUG_OUTPUT
    ret = get_mem_object_type() + ": this=" + itoa(reinterpret_cast<int64_t>(this)) +
            "\n         ; callid=" + get_call_id() +
            "; peerUri=" + peer_uri;

    ret += "\n          ";
    if( muted_senders )
        ret += "; muted_senders = true";
    else
        ret += "; muted_senders = false";

    ret += "\n          ";
    if( silenced_sources )
        ret += "; silenced_sources = true";
    else
        ret += "; silenced_sources = false";

    if( call_recorder )
    {
        SRef<Call_Recorder *> cr = dynamic_cast<Call_Recorder *>(*call_recorder);
        ret += "\n          ";
        cr = dynamic_cast<Call_Recorder *>( *call_recorder );
        ret += "; " + cr->get_debug_string();
    }else
        ret += "\n          (no call recorder)";

    for( std::list< SRef<Realtime_Media_Stream_Receiver *> >::iterator it = realtime_media_stream_receivers.begin();
         it != realtime_media_stream_receivers.end(); it++ )
    {
        ret += "\n          " + (*it)->get_debug_string();
    }
    for( std::list< SRef<Realtime_Media_Stream_Sender *> >::iterator it2 =  realtime_media_stream_senders.begin();
         it2 !=  realtime_media_stream_senders.end(); it2++ )
    {
        ret += "\n          " + (*it2)->get_debug_string();
    }
#endif
    return ret;
}

void Session::clear_realtime_media_stream_receivers()
{
    realtime_media_stream_receivers.clear();
}

const std::string &Session::get_peer_uri() const
{
    return peer_uri;
}

void Session::add_streams()
{
    Realtime_Media_Stream_Senders::iterator i;
    Realtime_Media_Stream_Senders::iterator last = realtime_media_stream_senders.end();
    if (!mikey) return;
    for( i = realtime_media_stream_senders.begin(); i != last; i++ ){
        uint32_t ssrc = (*i)->get_ssrc();
        mikey->add_sender( ssrc );
    }
}

pid_t Session::get_pid()
{
    return pid;
}

void  Session::set_pid(  pid_t p)
{
    pid = p;
}

SRef<Sdp_Packet *> Session::get_last_sdp_packet()
{
    return last_sdp_packet;
}

void Session::set_last_sdp_packet ( SRef <Sdp_Packet *> p )
{
    last_sdp_packet =  p ;
}

void Session::set_sdp_offer_200OK(SRef < Sdp_Packet *> pckt )
{
    set_last_sdp_packet( pckt);
    sdp_answer = pckt;
}

void Session::add_realtime_media_stream_receiver_participant(SRef <Sdp_HeaderM *> headerM , int index, SRef< Sip_Dialog *> voipForwarder , int mlineForwarder )
{
    list< SRef<Realtime_Media_Stream_Receiver *> >::iterator it;
    it = realtime_media_stream_receivers.begin();
    for (; it != realtime_media_stream_receivers.end(); it++)
    {
        if ((*it)->get_position_media_line() == index)
        {
            return; // a rtp receiver and sender has already been created for that media line
        }
    }

    SRef<Realtime_Media* > rtm;
    SRef<Rtp_Stream*> rtpStream4;
    SRef<Rtp_Stream*> rtpStream6;
    //                   SRef<UDPSocket *> sock;
    //                   SRef<UDPSocket *> sock6;
    SRef <Rtp_Receiver *> rtpReceiver;
    SRef <Rtp_Receiver *> rtp6Receiver;
    std::list< SRef<Media *> > media = get_media_list();
    std::list< SRef<Media *> > ::iterator hh;
    for (hh = media.begin(); hh != media.end(); hh++)
    {
        if ((*hh)->get_sdp_media_type().compare(headerM->get_media()) == 0)
        {
            /* we have iterated the media that we support (video and audio)
                 and we have found the media type that this media line waits answer

                so we construct one streamReceiver for that media that we found and we compare it with the charcteristics of the media line */
            SRef<Media *> media = *hh;
            rtm = dynamic_cast<Realtime_Media*> (*media);
            if (!rtm) cerr << "=================================== > ERROR rtm is NULL \n";
            SRef < Ip_Provider *> ipProvider = get_ip4_provider();
            SRef < Ip_Provider *> ip6Provider = get_ip6_provider();
            if (ipProvider)
                rtpReceiver = new Rtp_Receiver(ipProvider, call_id, streams_playerr, this);

            if (ip6Provider)
                rtp6Receiver = new Rtp_Receiver(ip6Provider, call_id, streams_playerr, this);

            if (rtpReceiver)
                rtpStream4 = rtpReceiver->get_rtp_stream();
            //sock = rtpReceiver->getSocket();
            if (rtp6Receiver)
                rtpStream6 = rtp6Receiver->get_rtp_stream();
            //sock6 = rtp6Receiver->getSocket();
            if (rtm->get_sdp_media_type().compare("video") == 0) {
                ;
                //this->setUDPSocket ( sock );
                //this->setUDPSocket6( sock6);
            }
        }
    }
    SRef <Realtime_Media_Stream_Receiver_Participant *> SRP = new Realtime_Media_Stream_Receiver_Participant(headerM, index, get_call_id(), rtm, this, rtpReceiver, rtp6Receiver, voipForwarder, mlineForwarder, this);
    SRef <Realtime_Media_Stream_Sender_Participant *> SSP = new Realtime_Media_Stream_Sender_Participant(headerM, index, get_call_id(), rtm, this, /*sock*/ rtpStream4, /*sock6*/ rtpStream6, streams_playerr);
    /* add to the senders list and the receivers list*/
    SRef < Realtime_Media_Stream_Receiver *> tmpReceiver = dynamic_cast<Realtime_Media_Stream_Receiver *> (*SRP);
    SRef < Realtime_Media_Stream_Sender *> tmpSender = dynamic_cast<Realtime_Media_Stream_Sender *> (*SSP);
    tmpReceiver->set_position_media_line(index);
    tmpSender->set_position_media_line(index);
    add_realtime_media_stream_sender(tmpSender);
    //realtime_media_stream_senders.push_back( *tmpSender );
    add_realtime_media_stream_receiver(tmpReceiver);
    //cerr<<"===============================> storing rtp receiver and sender for media line :: " << index <<" \n";
    if (headerM->get_port() == 0)
    {
        //cerr<<"===============================> stoting (at the same time )rtp receiver and sender for media line :: " << index <<" \n";
        tmpReceiver->recvonly = false;
        tmpSender->sendonly = true;
    }
}

void Session::add_realtime_media_stream_receiver_participant_top(SRef <Sdp_HeaderM *> headerM , int index, SRef< Sip_Dialog *> voipForwarder , int mlineForwarder )
{
    list< SRef<Realtime_Media_Stream_Receiver *> >::iterator it;
    it = realtime_media_stream_receivers.begin();
    for( ; it != realtime_media_stream_receivers.end(); it++ )
    {
        if ( (*it)->get_position_media_line() == index) {
            return ;// a rtp receiver and sender has already been created for that media line
        }
    }

    SRef<Realtime_Media* > rtm;
    SRef<Rtp_Stream*> rtpStream4;
    SRef<Rtp_Stream*> rtpStream6;
    //		   SRef<UDPSocket *> sock;
    //                 SRef<UDPSocket *> sock6;
    SRef <Rtp_Receiver *> rtpReceiver;
    SRef <Rtp_Receiver *> rtp6Receiver;
    std::list< SRef<Media *> >  media = get_media_list ();
    std::list< SRef<Media *> >  :: iterator hh;
    for( hh = media.begin(); hh != media.end(); hh++ ){
        if ( (*hh)->get_sdp_media_type().compare(headerM->get_media()) == 0 ){
            /* we have iterated the media that we support (video and audio)
                                     and we have found the media type that this media line waits answer
                                    so we construct one streamReceiver for that media that we found and we compare it with the charcteristics of the media line */
            SRef<Media *> media = *hh;
            rtm = dynamic_cast<Realtime_Media*>(*media);
            if ( ! rtm ) cerr<<"=================================== > ERROR rtm is NULL \n";
            SRef < Ip_Provider *> ipProvider = get_ip4_provider();
            SRef < Ip_Provider *> ip6Provider = get_ip6_provider();
            if( ipProvider )
                rtpReceiver = new Rtp_Receiver( ipProvider, call_id, streams_playerr, this );

            if( ip6Provider )
                rtp6Receiver = new Rtp_Receiver( ip6Provider, call_id, streams_playerr, this );

            if( rtpReceiver )
                rtpStream4 = rtpReceiver->get_rtp_stream();
            //sock = rtpReceiver->getSocket();
            if( rtp6Receiver )
                rtpStream6 = rtpReceiver->get_rtp_stream();
            //sock6 = rtp6Receiver->getSocket();
            if( rtm->get_sdp_media_type().compare("video")  == 0 ) {;
                //this->setUDPSocket ( sock );
                //this->setUDPSocket6( sock6);
            }

        }
    }

    SRef <Realtime_Media_Stream_Receiver_Participant *> SRP = new Realtime_Media_Stream_Receiver_Participant(headerM,index,get_call_id(),rtm, this, rtpReceiver, rtp6Receiver, voipForwarder, mlineForwarder, this);
    SRef <Realtime_Media_Stream_Sender_Participant *> SSP = new Realtime_Media_Stream_Sender_Participant (headerM, index, get_call_id(), rtm, this, rtpStream4, rtpStream6, streams_playerr);
    /* add to the senders list and the receivers list*/
    SRef < Realtime_Media_Stream_Receiver *> tmpReceiver = dynamic_cast< Realtime_Media_Stream_Receiver  *> ( *SRP);
    SRef < Realtime_Media_Stream_Sender *> tmpSender = dynamic_cast< Realtime_Media_Stream_Sender  *> ( *SSP);
    tmpReceiver->set_position_media_line(index);
    tmpSender->set_position_media_line(index);
    add_realtime_media_stream_sender_top(tmpSender);
    //realtime_media_stream_senders.push_front( *tmpSender);
    add_realtime_media_stream_receiver_top( tmpReceiver);
    //cerr<<"===============================> storing rtp receiver and sender for media line :: " << index <<" \n";
    if ( headerM->get_port()== 0 )
    {
        //	cerr<<"===============================> stoting (at the same time )rtp receiver and sender for media line :: " << index <<" \n";
        tmpReceiver->recvonly = false;
        tmpSender->sendonly = false;
    }
}

void Session::add_realtime_media_stream_receiver_top( SRef<Realtime_Media_Stream_Receiver *> realtimeMediaStream )
{
    realtime_media_stream_receivers.push_front( *realtimeMediaStream );
    silence_sources( silenced_sources );
}

void Session::add_realtime_media_stream_sender_top( SRef<Realtime_Media_Stream_Sender *> realtimeMediaStream )
{
    realtime_media_stream_senders_lock.lock();
    realtimeMediaStream->set_muted( muted_senders );
    realtime_media_stream_senders.push_front( *realtimeMediaStream );
    realtime_media_stream_senders_lock.unlock();
}

void Session::set_zero_participants(int index)
{
    list<SRef<Realtime_Media_Stream_Sender *> >::iterator iSStream;
    list<SRef<Realtime_Media_Stream_Receiver *> >::iterator iRStream;


    realtime_media_stream_senders_lock.lock();
    for (iSStream = realtime_media_stream_senders.begin(); iSStream != realtime_media_stream_senders.end(); iSStream++)
    {
        if(index == (*iSStream)->get_position_media_line() )
        {
            SRef <Realtime_Media_Stream_Sender_Participant *> SSP = dynamic_cast< Realtime_Media_Stream_Sender_Participant * > (*(*iSStream));
            SSP->get_media_header()->set_port(0);
        }

    }
    realtime_media_stream_senders_lock.unlock();
    for (iRStream = realtime_media_stream_receivers.begin(); iRStream != realtime_media_stream_receivers.end(); iRStream++)
    {
        if(index == (*iRStream)->get_position_media_line() )
        {
            SRef <Realtime_Media_Stream_Receiver_Participant *> receiverParticipant = dynamic_cast< Realtime_Media_Stream_Receiver_Participant * > (*(*iRStream));
            receiverParticipant->get_media_header()->set_port(0);
        }

    }
}

SRef<Sdp_HeaderM* > Session::set_direction_attribute(SRef < Sdp_HeaderM *> offerHdr , SRef < Sdp_HeaderM *> ansHdr ,  SRef < Realtime_Media_Stream_Receiver * > recv, int index )
{
    list<SRef<Realtime_Media_Stream_Sender *> >::iterator iSStream;
    std :: string direction ;
    direction = offerHdr->get_direction_attribute();
    if ( direction.compare(" ") == 0  )
    {
        realtime_media_stream_senders_lock.lock();
        for (iSStream = realtime_media_stream_senders.begin(); iSStream != realtime_media_stream_senders.end(); iSStream++) \
        {
            if(index == (*iSStream)->get_position_media_line() )
            {
                (*iSStream)->sendonly =true ;
            }
        }
        realtime_media_stream_senders_lock.unlock();
        recv->recvonly = true;
        recv->_direction_attribute.assign("sendrecv");
        if ( ansHdr ) ansHdr->set_direction_attribute("sendrecv");
        return ansHdr;
    }
    if ( direction.compare("a=sendonly") == 0  )
    {
        /* the sender must stop sending */
        realtime_media_stream_senders_lock.lock();
        for (iSStream = realtime_media_stream_senders.begin(); iSStream != realtime_media_stream_senders.end(); iSStream++)
        {
            if(index == (*iSStream)->get_position_media_line() )
            {
                (*iSStream)->sendonly =false ;
                //(*iSStream)->stopSending();
            }
        }
        realtime_media_stream_senders_lock.unlock();
        recv->recvonly=true;
        /*restart receiver perhaps it was stopped previously */
        //recv->start();

        recv->_direction_attribute.assign("recvonly");
        if ( ansHdr )  ansHdr->set_direction_attribute("recvonly");
    }
    if ( direction.compare("a=recvonly") == 0)
    {
        recv->recvonly=false;

        realtime_media_stream_senders_lock.lock();
        for (iSStream = realtime_media_stream_senders.begin(); iSStream != realtime_media_stream_senders.end(); iSStream++)
        {
            if(index == (*iSStream)->get_position_media_line() )
            {
                /* starting the sender again perhaps it was stopped previously */
                (*iSStream)->sendonly =true ;
                //(*iSStream)->start();
            }
        }
        realtime_media_stream_senders_lock.unlock();

        recv->_direction_attribute.assign("sendonly");
        //recv->stopReceiving();
        if ( ansHdr ) ansHdr->set_direction_attribute("sendonly");
    }

    if ( direction.compare("a=sendrecv") == 0 )
    {
        realtime_media_stream_senders_lock.lock();
        for (iSStream = realtime_media_stream_senders.begin(); iSStream != realtime_media_stream_senders.end(); iSStream++)
        {
            if(index == (*iSStream)->get_position_media_line() )\
            {
                (*iSStream)->sendonly =true ;
            }
        }
        realtime_media_stream_senders_lock.unlock();
        recv->recvonly = true;
        recv->_direction_attribute.assign("sendrecv");
        if ( ansHdr ) ansHdr->set_direction_attribute("sendrecv");
    }
    if ( direction.compare("a=inactive") == 0 )
    {
        realtime_media_stream_senders_lock.lock();
        for (iSStream = realtime_media_stream_senders.begin(); iSStream != realtime_media_stream_senders.end(); iSStream++)
        {
            if(index == (*iSStream)->get_position_media_line() )
            {
                (*iSStream)->sendonly =false ;
                //(*iSStream)->stopSending();
            }
        }
        realtime_media_stream_senders_lock.unlock();
        recv->recvonly = false;
        //recv->stopReceiving();
        recv->_direction_attribute.assign("inactive");
        if ( ansHdr ) ansHdr->set_direction_attribute("inactive");
    }

    return ansHdr;
}

bool Session::check_mikey_in_offer( SRef<Sdp_Packet *> offer, std :: string peerUri )
{
    string keyMgmtMessage;
    this->peer_uri = peerUri;

    keyMgmtMessage = offer->get_session_level_attribute( "key-mgmt" );

    if( keyMgmtMessage != "" )
    {
        Mikey_Config *config = new Mikey_Config( identity );
        // FIXME free config
        mikey = new Mikey( config );
        // perhaps I should uncomment that
        add_streams(); //TODO: This only adds SRTP streams, no reliable media is handled.
        if( !mikey->responder_authenticate( keyMgmtMessage, peerUri ) ){
            error_string =  "Incoming key management message could not be authenticated";
            error_string += mikey->auth_error();
            //	cerr<<"=================================> error with the mkey aggrement offer will not be accepted\n"<<error_string<<"\n" ;
            return false;
        }
        this->peer_uri = mikey->peer_uri();
        mikey->set_mikey_offer();
    }
    else{
        /*securityConfig.*/ka_type = KEY_MGMT_METHOD_NULL;
    }

    return true;
}

bool Session::check_mikey_in_answer( SRef <Sdp_Packet *> answer , std :: string peerUri)
{
    SRef<Realtime_Media_Stream_Receiver *> receiver;
    this->peer_uri = peerUri;
#ifdef DEBUG_OUTPUT
    //      cerr << "Session::setSdpAnswer" << endl;
#endif
    if( mikey )
    {
        /* get the keymgt: attribute */
        string keyMgmtMessage =
                answer->get_session_level_attribute( "key-mgmt" );

        if( !peerUri.empty() ){
            mikey->get_key_agreement()->set_peer_uri( peerUri );
        }

        if( !mikey->initiator_authenticate( keyMgmtMessage ) )
        {
            error_string = "Could not authenticate the key management message";
            fprintf( stderr, "Auth failed\n");
            return false;
        }
        this->peer_uri = mikey->peer_uri();

        string mikeyErrorMsg = mikey->initiator_parse();
        if( mikeyErrorMsg != "" )
        {
            error_string = "Could not parse the key management message. ";
            error_string += mikeyErrorMsg;
            fprintf( stderr, "Parse failed\n");
            return false;
        }
    }
    return true;
}

SRef<Realtime_Media_Stream_Sender *>  Session::get_sender_at_media_line(int line )
{
    list< SRef<Realtime_Media_Stream_Sender *> >::iterator i;
    realtime_media_stream_senders_lock.lock();
    i = realtime_media_stream_senders.begin();
    for( ; i != realtime_media_stream_senders.end(); i++ )
    {
        if ( (*i)->get_position_media_line() == line )
        {
            realtime_media_stream_senders_lock.unlock();
            //cerr<<"==================> Found Sender at media line : " << (*i)->get_position_media_line() <<" \n";
            return (*i);
        }

    }
    realtime_media_stream_senders_lock.unlock();
    return NULL;
}

SRef <Sdp_HeaderM* > Session::set_receiver_connection_data (SRef <Sdp_HeaderM* > headerM , int mediaLine)
{
    SRef <Sdp_HeaderM *> newtmp;
    list< SRef<Realtime_Media_Stream_Receiver *> >::iterator i;
    i = realtime_media_stream_receivers.begin();
    for( ; i != realtime_media_stream_receivers.end(); i++ )
    {
        if ( (*i)->get_position_media_line() == mediaLine )
        {
            string ipString;
            string addrtype;
            if( !local_ip_string.empty() )
            {
                ipString = local_ip_string;
                addrtype = "IP4";
            }
            else{
                ipString = local_ip6_string;
                addrtype = "IP6";
            }
            SRef<Sdp_HeaderC*> c = new Sdp_HeaderC("IN", addrtype, ipString );
            headerM->set_connection(c);
            headerM->set_port( (*i)->get_port( addrtype ) );
            SRef < Realtime_Media_Stream_Receiver_Participant * > recvPa = dynamic_cast < Realtime_Media_Stream_Receiver_Participant *> (*(*i));
            if ( recvPa ) (recvPa)->set_media_header(headerM);
        }
    }
    return headerM;
}

void Session::set_sender_destination_param ( SRef <Sdp_HeaderM* > headerM , int mediaLine)
{
    SRef<IPAddress *> remoteAddress;
    SRef<Sdp_HeaderC *> c = headerM->get_connection();
    remoteAddress = c->get_ipadress();

    list< SRef<Realtime_Media_Stream_Sender *> >::iterator i;
    realtime_media_stream_senders_lock.lock();
    i = realtime_media_stream_senders.begin();
    for( ; i != realtime_media_stream_senders.end(); i++ )
    {
        if ( (*i)->get_position_media_line() == mediaLine )
        {
            realtime_media_stream_senders_lock.unlock();
            (*i)->set_port((uint16_t) headerM->get_port());
            (*i)->set_remote_address(remoteAddress);
            break;
        }
    }
}

int32_t  Session::get_receiver_port ( int mediaLine )
{
    list< SRef<Realtime_Media_Stream_Receiver *> >::iterator i;
    i = realtime_media_stream_receivers.begin();
    for( ; i != realtime_media_stream_receivers.end(); i++ )
    {
        if ( (*i)->get_position_media_line() == mediaLine )
            return (*i)->get_port();
    }
    return -1;
}

void Session::storing_media_line (SRef <Sdp_HeaderM* > headerM , int mediaLine)
{
    list< SRef<Realtime_Media_Stream_Receiver *> >::iterator i;
    i = realtime_media_stream_receivers.begin();
    for( ; i != realtime_media_stream_receivers.end(); i++ )
    {
        if ( (*i)->get_position_media_line() == mediaLine )
        {
            SRef < Realtime_Media_Stream_Receiver_Participant * > recvPa = dynamic_cast < Realtime_Media_Stream_Receiver_Participant *> (*(*i));
            if ( recvPa ) (recvPa)->set_media_header(headerM);
        }
    }
}

SRef<Sdp_Packet *> Session::get_sdp_packet_empty()
{
    string keyMgmtMessage;
    const char *transport = NULL;
    SRef<Sdp_Packet *> result;

    result = empty_sdp();

    if( identity->security_enabled )
    {
        int type = 0;
        // FIXME
        switch( ka_type )
        {
        case KEY_MGMT_METHOD_MIKEY_DH:
            type = KEY_AGREEMENT_TYPE_DH;
            break;
        case KEY_MGMT_METHOD_MIKEY_PSK:
            type = KEY_AGREEMENT_TYPE_PSK;
            break;
        case KEY_MGMT_METHOD_MIKEY_PK:
            type = KEY_AGREEMENT_TYPE_PK;
            break;
        case KEY_MGMT_METHOD_MIKEY_DHHMAC:
            type = KEY_AGREEMENT_TYPE_DHHMAC;
            break;
        case KEY_MGMT_METHOD_MIKEY_RSA_R:
            type = KEY_AGREEMENT_TYPE_RSA_R;
            break;
        default:
            mikey = NULL;
            return NULL;
        }

        SRef<Sdp_HeaderA *> a;
        Mikey_Config *config = new Mikey_Config( identity );
        // FIXME free config
        mikey = new Mikey( config );

        add_streams();

        keyMgmtMessage = mikey->initiator_create( type, peer_uri );
        if( mikey->error() )
        {
            //cerr<<"===========  something went wrong\n";
            return NULL;
        }
        result->set_session_level_attribute( "key-mgmt", keyMgmtMessage );
        transport = "RTP/SAVP";
    }
    else{
        transport = "RTP/AVP";
    }

    //set_last_sdp_packet(result);
    return result;
}

void Session::stop_streamers_at_line(int mediaLine)
{
    list< SRef<Realtime_Media_Stream_Sender *> >::iterator i;
    realtime_media_stream_senders_lock.lock();
    i = realtime_media_stream_senders.begin();
    for( ; i != realtime_media_stream_senders.end(); i++ )
    {
        if ( (*i)->get_position_media_line() == mediaLine )
        {
            realtime_media_stream_senders_lock.unlock();
            // (*i)->sendonly = false;
            (*i)->stop();
            break;
        }
    }

    list< SRef<Realtime_Media_Stream_Receiver *> >::iterator it;
    it = realtime_media_stream_receivers.begin();
    for( ; it != realtime_media_stream_receivers.end(); it++ )
    {
        if ( (*it)->get_position_media_line() == mediaLine )
        {
            //(*it)->recvonly = false;
            (*it)->stop();
            break;
        }

    }
}

SRef<Realtime_Media_Stream_Sender *>  Session::create_real_time_media_streams ( int index , std :: string mediaType,std:: string direction )
{
    SRef < Ip_Provider *> ipProvider = get_ip4_provider();
    SRef < Ip_Provider *> ip6Provider = get_ip6_provider();
    SRef <Rtp_Receiver *> rtpReceiver;
    SRef<Rtp_Receiver *> rtp6Receiver;
    std::list< SRef<Media *> >  media = get_media_list ();
    std::list< SRef<Media *> >  :: iterator mediaIterator;
    SRef<Realtime_Media_Stream_Sender *> sStream = NULL;
    SRef<Realtime_Media_Stream_Receiver *> rStream = NULL;

    for( mediaIterator = media.begin(); mediaIterator != media.end(); mediaIterator++ )
    {
        if ( (*mediaIterator)->get_sdp_media_type().compare(mediaType) == 0 )
        {
            SRef<Media *> media = *mediaIterator;
            SRef<Realtime_Media*> rtm = dynamic_cast<Realtime_Media*>(*media);
            if( ipProvider ) rtpReceiver = new Rtp_Receiver( ipProvider, call_id, streams_playerr, this );
            if( ip6Provider ) rtp6Receiver = new Rtp_Receiver( ip6Provider, call_id, streams_playerr, this );
            SRef<Rtp_Stream*> rtpStream4;
            SRef<Rtp_Stream*> rtpStream6;
            if( rtpReceiver ) rtpStream4 = rtpReceiver->get_rtp_stream();
            if( rtp6Receiver ) rtpStream6 = rtp6Receiver->get_rtp_stream();
            if( rtm->get_sdp_media_type().compare("video")  == 0 ) {;
                //this->setUDPSocket ( sock );
                //this->setUDPSocket6( sock6);
            }

            sStream = new Realtime_Media_Stream_Sender( call_id, rtm, this, rtpStream4, rtpStream6, streams_playerr);
            sStream->set_position_media_line(index);

            realtime_media_stream_senders_lock.lock();
            realtime_media_stream_senders.push_back( *sStream );
            realtime_media_stream_senders_lock.unlock();

            if( !rtpReceiver && !ipProvider.is_null() )rtpReceiver = new Rtp_Receiver( ipProvider, call_id, streams_playerr, this );
            if( !rtp6Receiver && !ip6Provider.is_null() )rtp6Receiver = new Rtp_Receiver( ip6Provider, call_id, streams_playerr, this );
            rStream = new Realtime_Media_Stream_Receiver( call_id, rtm, this, rtpReceiver, rtp6Receiver, this );
            rStream->set_peer_uri(this->peer_uri);
            /* no medi line has assinged to that streamer*/
            rStream->set_position_media_line(index);
            this->add_realtime_media_stream_receiver( rStream );
            rStream->_direction_attribute.assign(direction);
            //	cerr<<"===========================> CreateRealTimeMEdia streams called for senders and receivers at : " << index << " \n";
            /* fixing directions attributes */
            if ( direction.compare("a=sendonly") == 0  )
            {
                sStream->sendonly = true;
                rStream->recvonly = false;
            }

            if ( direction.compare("a=recvonly") == 0  )
            {
                sStream->sendonly = false;
                rStream->recvonly = true;
            }
            if ( direction.compare("a=sendrecv") == 0  )
            {
                sStream->sendonly = true;
                rStream->recvonly = true;
            }
            if ( direction.compare("a=inactive") == 0  )
            {
                sStream->sendonly = false;
                rStream->recvonly = false;
            }
        }
    }
    return sStream;
}

void Session::clear_streamers()
{
    std::list< SRef<Realtime_Media_Stream_Receiver *> >::iterator it;
    it = realtime_media_stream_receivers.begin();
    for( ; it != realtime_media_stream_receivers.end(); it++ )
    {
        (*it)->stop();
    }
    realtime_media_stream_receivers.clear();

    realtime_media_stream_senders_lock.lock();
    std::list< SRef<Realtime_Media_Stream_Sender *> >::iterator i;
    i = realtime_media_stream_senders.begin();
    for( ; i != realtime_media_stream_senders.end(); i++ )
    {
        (*i)->stop();
    }

    realtime_media_stream_senders.clear();
    realtime_media_stream_senders_lock.unlock();
}

SRef<Media*> Session::get_media_of_type(std::string sdpmediatype)
{
    std::list< SRef<Media *> >::iterator i;
    for (i = media.begin(); i != media.end();i++)
    {
        if ( (*i)->get_sdp_media_type() == sdpmediatype)
            return *i;
    }
    return NULL;
}

void Session::video_keyframe_request_arrived()
{
#ifdef VIDEO_SUPPORT
    //	cout << "Session::video_keyframe_request_arrived() called" << endl;
    for(auto it=realtime_media_stream_senders.begin(), end = realtime_media_stream_senders.end(); it!=end; ++it)
    {
        Video_Media *videoMedia = dynamic_cast<Video_Media *>(*(*it)->get_realtime_media());
        if(videoMedia)
        {
            videoMedia->keyframe_request_arrived();
        }
    }
#endif
}

void Session::set_keyframe_request_callback(IRequest_Video_Keyframe *_keyframe_request_callback)
{
    keyframeRequestCallback = _keyframe_request_callback;
}

void Session::try_requesting_video_keyframe()
{
    keyframeRequestMutex.lock();
    keyframeRequestCalled = true;
    keyframeRequestCondVar.broadcast();
    keyframeRequestMutex.unlock();
}

void Session::run()
{
#ifdef DEBUG_OUTPUT
    set_thread_name("Session::run");
    cout << "Keyframe request thread started" << endl;
#endif
    bool condVarDeleted = false;
    while(!keyframeRequestThreadQuit && !condVarDeleted)
    {
        keyframeRequestMutex.lock();
        if(!keyframeRequestCalled)
            Cond_Var::wait(keyframeRequestCondVar, keyframeRequestMutex, condVarDeleted);
        keyframeRequestCalled = false;
        keyframeRequestMutex.unlock();
        if(!keyframeRequestThreadQuit && !condVarDeleted)
        {
            if(keyframeRequestCallback)
                keyframeRequestCallback->try_requesting_video_keyframe();
            my_sleep(KEYFRAME_REQUEST_MIN_INTERVAL_ms);
        }
    }
}

Mini_Sip* Session::get_mini_sip()
{
    return minisip;
}

SRef<Message_Router*> Session::get_message_router()
{
    return minisip->get_message_router();
}
