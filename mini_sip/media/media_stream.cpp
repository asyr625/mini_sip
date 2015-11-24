#include "media_stream.h"
#if 1

#include "session.h"

#include "mikey_payload_sp.h"
#include "key_agreement.h"

#include "sdp_headerm.h"
#include "sdp_headera.h"
#include "sdp_packet.h"

#include "udp_socket.h"
#include "string_utils.h"
#include "timestamp.h"

#include "media.h"
#include "rtp_receiver.h"
#include "codec.h"
#include "ip_provider.h"
#include "rtcp_report_fir.h"
#include "rtcp_report_app_view.h"
#include "reliable_media.h"

#include<iostream>
#ifdef VIDEO_SUPPORT
#include "video_media.h"
#endif


#ifdef _WIN32_WCE
#	include "minisip_wce_extra_includes.h"
#endif

int video_name_counter = 1;
using namespace std;


Media_Stream::Media_Stream(std::string callId_, SRef<Media*> m, SRef<Session*> s)
    : _call_id(callId_), _media(m), _session(s)
{
}

std::string Media_Stream::get_debug_string()
{
    std::string ret;
#ifdef DEBUG_OUTPUT
    ret = get_mem_object_type() + " this=" + itoa(reinterpret_cast<uint64_t>(this)) +
            " media="+media->get_sdp_media_type()+" port=" + itoa(get_port());
#endif
    return ret;
}

std::string Media_Stream::get_sdp_media_type() const
{
    my_assert( _media );
    if( _media )
    {
        return _media->get_sdp_media_type();
    }
    return "";
}

std::list<std::string> Media_Stream::get_sdp_attributes()
{
    return _media->get_sdp_attributes();
}

std::string Media_Stream::get_call_id() const
{
    return _call_id;
}

SRef<Media *> Media_Stream::get_media() const
{
    return _media;
}

SRef<Session*> Media_Stream::get_session() const
{
    return _session;
}
Reliable_Media_Stream::Reliable_Media_Stream(std::string callId, SRef<Reliable_Media*> m, SRef<Session*> s)
    : Media_Stream(callId, *m, s)
{
}

Realtime_Media_Stream::Realtime_Media_Stream( std::string callId, SRef<Realtime_Media *> m, SRef<Session*> s)
    : Media_Stream(callId, *m, s),
      _realtime_media(m) /*callId(cid), media(m)*/, _ka(NULL)
{
    _disabled = false;
    _direction_attribute.assign("sendrecv");
#ifdef ZRTP_SUPPORT
    zrtp_bridge = NULL;
#endif
}


bool parseRtpMap(string rtpMap, string &name, string &rate, string &param)
{
    size_t pos = rtpMap.find('/');

    if( pos == string::npos )
        return false;

    size_t pos2 = rtpMap.find('/', pos + 1);

    name = rtpMap.substr(0, pos);
    rate = rtpMap.substr(pos + 1, pos2 - pos - 1);

    if( pos2 != string::npos )
        param = rtpMap.substr(pos2 + 1);
    else
        param = "1";

    return true;
}

bool Realtime_Media_Stream::matches( SRef<Sdp_HeaderM *> m, uint32_t formatIndex )
{
    string sdpRtpMap;
    string sdpFmtpParam;

    //	int i;
    string sdpPayloadType = m->get_format( formatIndex );

    _realtime_media->handle_mheader( m );

    // pn507 This checks for "Audio"
    if( m->get_media() != get_sdp_media_type() )
        return false;

    sdpRtpMap = m->get_rtp_map( sdpPayloadType );
    sdpFmtpParam = m->get_fmtp_param( sdpPayloadType );

    std::list<SRef<Codec_Description *> > &codecs = _realtime_media->get_available_codecs();
    std::list<SRef<Codec_Description *> >::iterator iC;
    string sdpName;
    string sdpRate;
    string sdpParam;

    parseRtpMap(sdpRtpMap, sdpName, sdpRate, sdpParam);

    for( iC = codecs.begin(); iC != codecs.end(); iC ++ )
    {
        string codecRtpMap;
        int codecPayloadType;

        codecRtpMap = (*iC)->get_sdp_media_attributes();
        codecPayloadType = (*iC)->get_sdp_media_type();
        if( (*iC)->get_codec_name() == "iLBC" )
        {
            if( sdpFmtpParam != "mode=20" ) { //iLBC only supports 20ms frames (in minisip)
                continue;
            } //else ... does not mean we accept it, it still goes through the normal checks ...
        }
        if( sdpRtpMap != "" && codecRtpMap != "" )
        {
            string codecName;
            string codecRate;
            string codecParam;

            if( !parseRtpMap(codecRtpMap, codecName, codecRate, codecParam) )
                continue;

            bool sdpRtpMapEqual = !str_case_cmp( codecName.c_str(), sdpName.c_str() ) && codecRate == sdpRate && codecParam == sdpParam;
            if ( sdpRtpMapEqual )
            {
                _local_payload_type = itoa(codecPayloadType);
                return true;
            }
            continue;
        }
        else
        {
            if( sdpPayloadType == itoa(codecPayloadType) )
            {
                _local_payload_type = itoa(codecPayloadType);
                return true;
            }
        }
    }
    return false;
}

SRef<Crypto_Context *> Realtime_Media_Stream::init_crypto( uint32_t ssrc, uint16_t seq_no )
{
    SRef<Crypto_Context *> cryptoContext;

    _ka_lock.lock();
    if( !_ka )
    {
        /* Dummy cryptocontext */
        cryptoContext = new Crypto_Context( ssrc );
    }
    else
    {

        unsigned char * masterKey = new unsigned char[16];
        unsigned char * masterSalt = new unsigned char[14];

        uint8_t  csId = ka->getSrtpCsId( ssrc );
        uint32_t roc = ka->getSrtpRoc( ssrc );
        uint8_t  policyNo = ka->findpolicyNo( ssrc );
        //Extract Srtp policy !!! Check the return value if type not available
        uint8_t ealg  = ka->getPolicyParamTypeValue(policyNo, MIKEY_PROTO_SRTP, MIKEY_SRTP_EALG);
        uint8_t ekeyl = ka->getPolicyParamTypeValue(policyNo, MIKEY_PROTO_SRTP, MIKEY_SRTP_EKEYL);
        uint8_t aalg  = ka->getPolicyParamTypeValue(policyNo, MIKEY_PROTO_SRTP, MIKEY_SRTP_AALG);
        uint8_t akeyl = ka->getPolicyParamTypeValue(policyNo, MIKEY_PROTO_SRTP, MIKEY_SRTP_AKEYL);
        uint8_t skeyl = ka->getPolicyParamTypeValue(policyNo, MIKEY_PROTO_SRTP, MIKEY_SRTP_SALTKEYL);
        //uint8_t prf   = ka->getPolicyParamTypeValue(policyNo, MIKEY_PROTO_SRTP, MIKEY_SRTP_PRF);	 //Not used
        uint8_t keydr = ka->getPolicyParamTypeValue(policyNo, MIKEY_PROTO_SRTP, MIKEY_SRTP_KEY_DERRATE);
        uint8_t encr  = ka->getPolicyParamTypeValue(policyNo, MIKEY_PROTO_SRTP, MIKEY_SRTP_ENCR_ON_OFF);
        //uint8_t cencr = ka->getPolicyParamTypeValue(policyNo, MIKEY_PROTO_SRTP, MIKEY_SRTCP_ENCR_ON_OFF);//Not used
        //uint8_t fecor = ka->getPolicyParamTypeValue(policyNo, MIKEY_PROTO_SRTP, MIKEY_SRTP_FEC_ORDER);	 //Not used
        uint8_t auth  = ka->getPolicyParamTypeValue(policyNo, MIKEY_PROTO_SRTP, MIKEY_SRTP_AUTH_ON_OFF);
        uint8_t autht = ka->getPolicyParamTypeValue(policyNo, MIKEY_PROTO_SRTP, MIKEY_SRTP_AUTH_TAGL);
        //uint8_t prefi = ka->getPolicyParamTypeValue(policyNo, MIKEY_PROTO_SRTP, MIKEY_SRTP_PREFIX);	 //Not used

#ifdef ENABLE_TS
        ts.save("TEK_START");
#endif

        ka->genTek( csId,  masterKey,  16 );

#ifdef ENABLE_TS
        ts.save("TEK_STOP");
#endif

        ka->genSalt( csId, masterSalt, 14 );

#ifdef DEBUG_OUTPUT
#if 0
        fprintf( stderr, "csId: %i\n", csId );
        cerr << "SSRC: "<< ssrc <<" - TEK: " << bin_to_hex( masterKey, 16 ) << endl;
        cerr << "SSRC: "<< ssrc <<" - SALT: " << bin_to_hex( masterSalt, 14 )<< endl;
#endif
#endif

        //		if( csId != 0 ){
        cryptoContext = new Crypto_Context( ssrc, roc, seq_no, keydr,
                                            ealg, aalg, masterKey, 16, masterSalt, 14, ekeyl, akeyl, skeyl, encr, auth, autht );

        cryptoContext->derive_srtp_keys( 0 );
        //		}
        //		else{
        //			cerr << "EEEE: WARNING - csId is NULL"<<endl;
        //			cryptoContext = new CryptoContext( ssrc );
        //		}
    }

    _crypto_contexts.push_back( cryptoContext );
    _ka_lock.unlock();
    return cryptoContext;
}

SRef<Crypto_Context *> Realtime_Media_Stream::get_crypto_context( uint32_t ssrc, uint16_t seq_no )
{
    _ka_lock.lock();
    list< SRef<Crypto_Context *> >::iterator i;

    for( i = _crypto_contexts.begin(); i!= _crypto_contexts.end(); i++ )
    {
        if( (*i)->get_ssrc() == ssrc )
        {
            _ka_lock.unlock();
            return (*i);
        }
    }

    _ka_lock.unlock();
    return init_crypto( ssrc, seq_no );
}

void Realtime_Media_Stream::set_key_agreement( SRef<Key_Agreement *> ka )
{
    _ka_lock.lock();
    this->_ka = ka;

    _crypto_contexts.clear();

    _ka_lock.unlock();
}

SRef<Realtime_Media *> Realtime_Media_Stream::get_realtime_media()
{
    return _realtime_media;
}


#ifdef ZRTP_SUPPORT
void Realtime_Media_Stream::set_key_agreement_zrtp(SRef<Crypto_Context *>cx)
{
    _ka_lock.lock();

    std::list< SRef<Crypto_Context *> >::iterator i;

    for( i = _crypto_contexts.begin(); i!= _crypto_contexts.end(); i++ )
    {
        if( (*i)->get_ssrc() == cx->get_ssrc() )
        {
            _crypto_contexts.erase(i);
            break;
        }
    }
    _crypto_contexts.push_back(cx);

    _ka_lock.unlock();
}

void Realtime_Media_Stream::set_zrtp_host_bridge(SRef<Zrtp_Host_Bridge_Minisip *> zsb)
{
    zrtp_bridge = zsb;
}

SRef<Zrtp_Host_Bridge_Minisip *> Realtime_Media_Stream::get_zrtp_host_bridge()
{
    return zrtp_bridge;
}
#endif


Realtime_Media_Stream_Sender::Realtime_Media_Stream_Sender( std::string callId, SRef<Realtime_Media *> m, SRef<Session*> s,
                                                            SRef<Rtp_Stream*> rtpStream4, SRef<Rtp_Stream*> rtpStream6,
                                                            IStreams_Player_Report_Timestamps *_streamsPlayer)
    : Realtime_Media_Stream( callId, m, s ),
      rtp_stream4(rtpStream4),
      rtp_stream6(rtpStream6)
{
    selected_encoder = NULL;
    running = false;
    sendonly = true ;
    remote_port = 0;
    seq_no = (uint16_t)rand();
    ssrc = rand();
    last_ts = rand();

    payload_type = "255";
    set_muted( true );
    receiver_forwarder_media_line = -1 ;
    mute_counter = 0;
    set_source_id ( 0 );

    if (!rtp_stream4){
        rtp_stream4 = new Rtp_Stream(false, callId, _streamsPlayer, *s);
    }
}

std::string Realtime_Media_Stream_Sender::get_debug_string()
{
    string ret;
#ifdef DEBUG_OUTPUT

    ret = get_mem_object_type() + " this=" + itoa(reinterpret_cast<uint64_t>(this)) +
            " media="+_media->get_sdp_media_type()+"; port=" + itoa(get_port()) +
            "; remotePort=" + itoa(remote_port);

    if( is_muted() == true )
        ret += "; isMuted=true";
    else ret += "; isMuted=false";
#endif
    return ret;
}

SRef<Encoder_Instance*> Realtime_Media_Stream_Sender::get_selected_encoder()
{
    return selected_encoder;
}
void Realtime_Media_Stream_Sender::start()
{
    if ( !running)
    {
        if (!_disabled)
        {
            _realtime_media->register_realtime_media_sender( this );
            running = true;
            set_muted(false);
        }
    }
}

void Realtime_Media_Stream_Sender::stop()
{
    _realtime_media->unregister_realtime_media_sender( this );
    rtp_stream4 = NULL;
    rtp_stream6 = NULL;
    //senderSock = NULL;
    //sender6Sock = NULL;
    sendonly = true;
    running = false;
#ifdef ZRTP_SUPPORT
    if (zrtp_bridge)
    {
        zrtp_bridge->stop();
    }
#endif
    running = false;
    sentPacketCount = 0;
}

void Realtime_Media_Stream_Sender::stop_sending()
{
    if ( running )
    {
        _realtime_media->unregister_realtime_media_sender( this );
        running = false ;
    }
}

void Realtime_Media_Stream_Sender::set_port( uint16_t port )
{
    remote_port = port;
}

uint16_t Realtime_Media_Stream_Sender::get_port()
{
    return remote_port;
}

uint16_t Realtime_Media_Stream_Sender::get_local_port()
{
    Rtp_Stream *rtpStream = (remote_address->get_address_family() == AF_INET6) ? *rtp_stream6 : *rtp_stream4;
    if(rtpStream)
    {
        SRef<UDP_Socket*> udpSocket = rtpStream->get_rtp_socket();
        if(udpSocket)
            return udpSocket->get_port();
    }
    return 0;
}

static bool first = true;

void Realtime_Media_Stream_Sender::send( byte_t * data, uint32_t length, uint32_t *givenTs, bool marker, bool dtmf)
{
    if (this->remote_address.is_null())
    {
        my_dbg("media") << " Realtime_Media_Stream_Sender::send called before " <<
                           "set_remote_address!" << endl;
        return;
    }

    SRef<SRtp_Packet *> packet;
    if (first){
#ifdef ENABLE_TS
        ts.save("rtp_send");
#endif
        first = false;
    }

    sender_lock.lock();
    if(givenTs == NULL)
        increase_last_ts();
    else
        last_ts = *givenTs;
    packet = new SRtp_Packet( data, length, ++seq_no, last_ts, ssrc );

    if( dtmf )
    {
        //std::cout << "RealtimeMediaStreamSender::send() packet is DTMF" << std::endl;
        packet->get_header().set_payload_type( 101 );
    }
    else
    {
        //		std::cout << "RealtimeMediaStreamSender::send() payload type is " << payloadType << std::endl;
        if( payload_type != "255" )
        {
            packet->get_header().set_payload_type( atoi(payload_type.c_str() ) );
        }
        else
        {
            //packet->get_header().setPayloadType( selectedCodec->getSdpMediaType() );
            packet->get_header().set_payload_type( 99 );
        }
    }
    if( marker )
    {
        //std::cout << "RealtimeMediaStreamSender::send() marker present" << std::endl;
        packet->get_header().set_marker( marker );
    }


    SRef<Processing_Data_Rtp*> rdata = new Processing_Data_Rtp(packet);
    //        rdata->rtpPackets.push_back(*packet);

    sender_lock.unlock();

    get_pipeline(ssrc)->process(*rdata);
}

void Realtime_Media_Stream_Sender::send( const SRef<Processing_Data*>& data )
{
    my_assert(selected_encoder);
    //1. encode
    SRef<Processing_Data_Rtp*> rdata = selected_encoder->encode(data, &seq_no , ssrc); //ts, seq, ssrc

    //2. send rtp packets through RTP pipeline
    if (rdata)
        get_pipeline(ssrc)->process(*rdata);
}


#ifdef ZRTP_SUPPORT
void Realtime_Media_Stream_Sender::send_zrtp(unsigned char* data, int length, unsigned char* payload, int payLen)
{
    if (this->remote_address.is_null())
    {
        my_dbg("media/zrtp") << " Realtime_Media_Stream_Sender::send_zrtp called before " <<
                                "set_remote_address!" << endl;
        return;
    }

    SRtp_Packet * packet;
    if (first){
#ifdef ENABLE_TS
        ts.save("rtp_send");
#endif
        first=false;
    }

    sender_lock.lock();
    uint32_t ts = time(NULL);
    packet = new SRtp_Packet(payload, payLen, zrtp_bridge->getZrtpSendSeqNo(),
                             ts, zrtp_bridge->getZrtpSendSsrc() );

    packet->get_header().set_payload_type(13);
    packet->set_ext_header(data, length);

    packet->protect(get_crypto_context(ssrc, seq_no));

    // packet->enableZrtpChecksum();
    send(*rtp_stream4, packet, **remote_address, remote_port);
    delete packet;
    sender_lock.unlock();
}
#endif

void Realtime_Media_Stream_Sender::send_rtp_packet(const SRef<Rtp_Packet*> & rtpPacket)
{
    if( remote_address->get_address_family() == AF_INET && rtp_stream4 )
        send(*rtp_stream4, rtpPacket, **remote_address, remote_port);
    else if( remote_address->get_address_family() == AF_INET6 && rtp_stream6 )
        send(*rtp_stream6, rtpPacket, **remote_address, remote_port);
}

void Realtime_Media_Stream_Sender::send_rtp_packet(const SRef<SRtp_Packet*> & rtpPacket, uint16_t seq_no, uint32_t packetSsrc)
{
    _realtime_media->lock_senders();
    rtpPacket->protect( get_crypto_context( packetSsrc, seq_no) );
    if( remote_address->get_address_family() == AF_INET && rtp_stream4)
    {
        send(*rtp_stream4, *rtpPacket, **remote_address, remote_port);
    }
    else if( remote_address->get_address_family() == AF_INET6 && rtp_stream6)
    {
        // cerr<<"now to ipv6 address:port "<< remote_address->get_string() <<":"<<remote_port <<"\n";
        send(*rtp_stream6, *rtpPacket, **remote_address, remote_port);
    }
    _realtime_media->unlock_senders();
}

void Realtime_Media_Stream_Sender::set_remote_address( SRef<IPAddress *> ra )
{
    my_dbg("media") << "Realtime_Media_Stream_Sender::set_remote_address: " <<
                       ra->get_string() << endl;
    this->remote_address = ra;
#ifdef ZRTP_SUPPORT
    if (zrtp_bridge) {
        zrtp_bridge->setRemoteAddress(ra);
    }
#endif
}

bool Realtime_Media_Stream_Sender::mute_keep_alive( uint32_t max)
{
    bool ret = false;

    //if muted, only return true if packet is keep alive
    if( is_muted() )
    {
        mute_counter++;
        if( mute_counter >= max )
        {
            ret = true;
            mute_counter = 0;
        }
    } else {
        //if active sender, let through
        ret = true;
    }
    return ret;
}

bool Realtime_Media_Stream_Sender::matches( SRef<Sdp_HeaderM *> m, uint32_t formatIndex )
{
    bool result = Realtime_Media_Stream::matches( m, formatIndex );

    if( result && !selected_encoder )
    {
        selected_encoder = _realtime_media->create_encoder_instance( atoi( _local_payload_type.c_str() )  );
        payload_type = itoa( (uint8_t)atoi(m->get_format( formatIndex ).c_str()) )  ;
        result = true;
    }

    return result;
}

uint32_t Realtime_Media_Stream_Sender::get_ssrc()
{
    return ssrc;
}


SRef<IPAddress *> Realtime_Media_Stream_Sender::get_local_address()
{
    Rtp_Stream *rtpStream = (remote_address->get_address_family() == AF_INET6) ? *rtp_stream6 : *rtp_stream4;
    if(rtpStream)
    {
        SRef<UDP_Socket*> udpSocket = rtpStream->get_rtp_socket();
        if(udpSocket)
            return udpSocket->get_local_address();
    }
    return SRef<IPAddress *>();
}

std::string Realtime_Media_Stream_Sender::get_encoder_description()
{
    if(selected_encoder)
    {
        SRef<Codec_Description*> codec = selected_encoder->get_codec();
        if(codec)
            return codec->get_codec_description();
    }
    return "?";
}

float Realtime_Media_Stream_Sender::get_sent_video_framerate_fps()
{
    return -1.0;
}

void Realtime_Media_Stream_Sender::handle_data(const SRef<Processing_Data*>& data )
{
    sender_lock.lock();

    SRef<Processing_Data_Rtp*> rtpdata = dynamic_cast<Processing_Data_Rtp*>(*data);

    for (auto i = rtpdata->_rtp_packets.begin();i != rtpdata->_rtp_packets.end(); i++)
    {
        SRef<SRtp_Packet *> packet = (*i);
        packet->protect( get_crypto_context( packet->get_header().get_ssrc(), packet->get_header().get_seq_no() ) );
        if( remote_address->get_address_family() == AF_INET && /*senderSock*/ rtp_stream4 )
        {
            send(*rtp_stream4, *packet, **remote_address, remote_port);
        }
        else if( remote_address->get_address_family() == AF_INET6 && rtp_stream6 )
        {
            send(*rtp_stream6, *packet, **remote_address, remote_port );
        }
    }
    sender_lock.unlock();

#ifdef ZRTP_SUPPORT
    if (zrtp_bridge && zrtp_bridge->getSsrcSender() == 0) {
        zrtp_bridge->setSsrcSender(ssrc);
        zrtp_bridge->start();
    }
#endif
}

void Realtime_Media_Stream_Sender::send_rtcp_packet(SRef<Rtcp_Packet*> p)
{
    if (rtp_stream4)
        rtp_stream4->get_rtcp_mgr()->send_rtcp(p);
    else if (rtp_stream6)
        rtp_stream6->get_rtcp_mgr()->send_rtcp(p);
    else
        my_assert(0);
}

void Realtime_Media_Stream_Sender::request_codec_intracoded()
{
    selected_encoder->request_codec_intracoded();
}

void Realtime_Media_Stream_Sender::send(Rtp_Stream* rtpStream, const SRef<Rtp_Packet *> &packet, const IPAddress &to_addr, const uint16_t &port)
{
    ++sentPacketCount;
    Rtp_Header &hdr = packet->get_header();
    totalSentPacketSize += hdr.size() + packet->get_content_length();
    rtpStream->send(packet, to_addr, port);
}

SRef<Realtime_Media_Pipeline*> Realtime_Media_Stream_Sender::get_pipeline(uint32_t ssrc)
{
    if (_ssrc_pipeline.count(ssrc) == 0)
    {
        SRef<Realtime_Media_Pipeline*> p = new Realtime_Media_Pipeline(get_session()->get_mini_sip(), get_session(), this, this, media->get_sdp_media_type(), PROCESSOR_DIRECTION_DEV2NET, PROCESSOR_INSERT_POINT_RTP);
        Processor_Registry::get_instance()->add_plugins(p, this);
        _ssrc_pipeline[ssrc]=p;
    }
    return _ssrc_pipeline[ssrc];
}


Realtime_Media_Stream_Receiver::Realtime_Media_Stream_Receiver( std::string callId,  SRef<Realtime_Media *> m, SRef<Session*> s,
                                                                SRef<Rtp_Receiver *> rtpRecv, SRef<Rtp_Receiver *> rtp6Recv,
                                                                IRequest_Video_Keyframe *keyframeRequestCallback)
    : Realtime_Media_Stream( callId,  m, s ),
      rtp_receiver( rtpRecv ),
      rtp6_receiver( rtp6Recv )
{
    id = rand();
    external_port = 0;
    running = false;
    recvonly = true;
    codec_list = m->get_available_codecs();
    initiated = false;
#ifdef VIDEO_SUPPORT
    Video_Media *videoMedia = dynamic_cast<Video_Media *>(*m);
    if(videoMedia)
        videoMedia->set_keyframe_request_callback(keyframeRequestCallback);
#endif
}

std::string Realtime_Media_Stream_Receiver::get_debug_string()
{
    string ret;
#ifdef DEBUG_OUTPUT
    ret = get_mem_object_type() + " this=" + itoa(reinterpret_cast<uint64_t>(this)) +
            " media="+media->get_sdp_media_type()+"; listening port=" + itoa(get_port());
    for( std::set<uint32_t>::iterator it = _ssrc_set.begin();
         it != _ssrc_set.end();
         it++) {
        "; ssrc=" + itoa((*it));
    }
    std::map<uint32_t, SRef<Realtime_Media_Pipeline*> >::iterator pi;
    for (pi= _ssrc_pipeline.begin(); pi!= _ssrc_pipeline.end(); pi++)
        ret+=" "+(*pi).second->get_debug_string();
#endif
    return ret;
}

void Realtime_Media_Stream_Receiver::send_rtcp_fir(uint32_t ssrc)
{
    SRef<Rtcp_Packet*> p = new Rtcp_Packet;
    Rtcp_Report_Fir* fir = new Rtcp_Report_Fir(ssrc);
    p->add_report(fir);
    send_rtcp_packet(p);
}

void Realtime_Media_Stream_Receiver::handle_data(const SRef<Processing_Data*>& data )
{
    SRef<Processing_Data_Rtp*> rdata = (Processing_Data_Rtp*)*data;
    std::list<SRef<SRtp_Packet*> >::iterator i;
    for (i = rdata->_rtp_packets.begin(); i != rdata->_rtp_packets.end(); i++)
    {
        SRef<SRtp_Packet*> packet = *i;
        _realtime_media->play_data( *packet );
    }
}

void Realtime_Media_Stream_Receiver::start()
{
    if( !running )
    {
        if ( !_disabled)
        {
            if( rtp_receiver )
                rtp_receiver->register_realtime_media_stream( this );

            if( rtp6_receiver )
                rtp6_receiver->register_realtime_media_stream( this );
            running = true;
        }
    }
}

void Realtime_Media_Stream_Receiver::stop_receiving()
{
    std::set<uint32_t>::iterator i;

    if  ( running )
    {
        if( rtp_receiver )
        {
            rtp_receiver->unregister_realtime_media_stream( this );
            rtp_receiver->stop();
        }
        if( rtp6_receiver )
        {
            rtp6_receiver->unregister_realtime_media_stream( this );
            rtp6_receiver->stop();
        }

        // putting on hold means that the call can start later with new ssrc parameters I am flashing
        // thus stoping sender however when I start again later is the grabber going to be open now it will stop
        ssrc_set_lock.lock();
        for( i = ssrc_set.begin(); i != ssrc_set.end(); i++ )
        {
            _realtime_media->unregister_media_source( *i );
        }
        ssrcSet.clear();
        ssrc_set_lock.unlock();
        running = false;
        // we do not flash the forwarders senders this port may have been on hold
        //forwarderSenders.clear();
    }
}

void Realtime_Media_Stream_Receiver::stop()
{
    std::set<uint32_t>::iterator i;
    initiated = true;
    _disabled = true;
    if( rtp_receiver )
    {
        rtp_receiver->unregister_realtime_media_stream( this );
        rtp_receiver->stop();
    }
    if( rtp6_receiver )
    {
        rtp6_receiver->unregister_realtime_media_stream( this );
        rtp6_receiver->stop();
    }
    if( rtp_receiver )
        rtp_receiver->join();

    if( rtp6_receiver )
        rtp6_receiver->join();

    ssrc_set_lock.lock();
    for( i = ssrc_set.begin(); i != ssrc_set.end(); i++ )
    {
        _realtime_media->unregister_media_source( *i );
    }
    ssrc_set.clear();
    ssrc_set_lock.unlock();
    //	forwarderSenders.clear();
    running = false;

    std::map<uint32_t, SRef<Realtime_Media_Pipeline*> >::iterator pi;
    for (pi = _ssrc_pipeline.begin(); pi != _ssrc_pipeline.end(); pi++)
        (*pi).second->stop();
    for (pi = _ssrc_pipeline.begin(); pi != _ssrc_pipeline.end(); pi++)
        (*pi).second->free();
}

uint16_t Realtime_Media_Stream_Receiver::get_port()
{
    return rtp_receiver ? rtp_receiver->get_port() : ( rtp6_receiver ? rtp6_receiver->get_port() : 0 );
}

uint16_t Realtime_Media_Stream_Receiver::get_rtcp_port()
{
    return rtp_receiver ? rtp_receiver->get_rtcp_port() : ( rtp6_receiver ? rtp6_receiver->get_rtcp_port() : 0 );
}

uint16_t Realtime_Media_Stream_Receiver::get_port( const std::string &addrType )
{
    if( addrType == "IP4" && rtp_receiver )
        return rtp_receiver->get_port();
    else if( addrType == "IP6" && rtp6_receiver )
        return rtp6_receiver->get_port();
    return 0;
}

uint16_t Realtime_Media_Stream_Receiver::get_rtcp_port( const std::string &addrType )
{
    if( addrType == "IP4" && rtp_receiver )
        return rtp_receiver->get_rtcp_port();
    else if( addrType == "IP6" && rtp6_receiver )
        return rtp6_receiver->get_rtcp_port();
    return 0;
}

void Realtime_Media_Stream_Receiver::handle_rtp_packet( const SRef<SRtp_Packet *> & packet, std::string callId, SRef<IPAddress *> from )
{
    uint32_t packetSsrc;
    uint16_t seq_no;

    //if packet is null, we had a read timeout from the rtpReceiver
    if( !packet )
        return;

    packetSsrc = packet->get_header().get_ssrc();
    seq_no = packet->get_header().get_seq_no();

#ifdef ZRTP_SUPPORT
    // if (zrtpBridge && packet->get_header().getExtension() && zrtpBridge->isZrtpPacket(packet)) {
    //    packet->checkZrtpChecksum(false);
    // }
    SRef<Crypto_Context *> pcc = get_crypto_context(packetSsrc, seq_no);

    // If empty crypto context for this SSRC but we are already in Secure
    // state then create a real CryptoContext for this SSRC. Assumption:
    // every SSRC stream sent via this connection is secured _and_ uses
    // the same crypto parameters.
    if (zrtp_bridge && zrtp_bridge->getSsrcSender() != 0 &&  zrtp_bridge->isSecureState() &&
            (pcc->getEalg() == MIKEY_SRTP_EALG_NULL))
    {
        pcc = zrtp_bridge->newCryptoContextForRecvSSRC(packetSsrc, 0, seq_no, 0L);
    }
    if( packet->unprotect(pcc)) { // Authentication or replay protection failed
        return;
    }
    if (zrtp_bridge && packet->get_header().get_extension() &&
            (zrtp_bridge->process_packet(packet) == 0)) {
        return;
    }
#else
    if( packet->unprotect( get_crypto_context( packetSsrc, seq_no ) )){
        // Authentication or replay protection failed
        return;
    }

#endif // ZRTP_SUPPORT

    got_ssrc( packetSsrc, callId );

    SRef<Processing_Data_Rtp*> data = new Processing_Data_Rtp(packet);
    get_pipeline(packetSsrc)->process(*data);
}

uint32_t Realtime_Media_Stream_Receiver::get_id()
{
    return id;
}

std::list<SRef<Codec_Description *> >& Realtime_Media_Stream_Receiver::get_available_codecs()
{
    return codec_list;
}

void Realtime_Media_Stream_Receiver::send_rtcp_app_view(unsigned subtype, uint32_t sender_ssrc, int width, int height)
{
    SRef<Rtcp_Packet*> rtcppkt = new Rtcp_Packet;
    Rtcp_Report_App_View * app = new Rtcp_Report_App_View( subtype, sender_ssrc, width, height );	// ssrc must be assigned to correct SSRC
    rtcppkt->add_report(app);

    send_rtcp_packet( rtcppkt );
}

#ifdef ZRTP_SUPPORT
void Realtime_Media_Stream_Receiver::handle_rtp_packet_ext(SRef<SRtp_Packet *> packet)
{
    uint32_t recvSsrc;
    uint16_t seq_no;

    //if packet is null, we had a read timeout from the rtpReceiver
    if( !packet )
        return;

    recvSsrc = packet->get_header().get_ssrc();
    seq_no = packet->get_header().get_seq_no();

    // if (zrtpBridge->isZrtpPacket(packet)) {
    //    packet->checkZrtpChecksum(false);
    // }
    // Look for a CryptoContext for this packet's SSRC
    SRef<Crypto_Context *> pcc = get_crypto_context(recvSsrc, seq_no);

    if( packet->unprotect(pcc)) { // Authentication or replay protection failed
        return;
    }
    zrtp_bridge->process_packet(packet);
}
#endif


void Realtime_Media_Stream_Receiver::send_rtcp_packet(SRef<Rtcp_Packet*> p)
{
    rtp_receiver->get_rtp_stream()->get_rtcp_mgr()->send_rtcp(p);
}

SRef<Realtime_Media_Pipeline*> Realtime_Media_Stream_Receiver::get_pipeline(uint32_t ssrc)
{
    if (_ssrc_pipeline.count(ssrc)==0)
    {
        SRef<Realtime_Media_Pipeline*> p = new Realtime_Media_Pipeline(_session->get_mini_sip(), session, this, this, _media->get_sdp_media_type(), PROCESSOR_DIRECTION_NET2DEV, PROCESSOR_INSERT_POINT_RTP);
        Processor_Registry::get_instance()->add_plugins(p, this);
        _ssrc_pipeline[ssrc] = p;
    }
    return _ssrc_pipeline[ssrc];
}

void Realtime_Media_Stream_Receiver::got_ssrc( uint32_t ssrc, std::string callId )
{
    ssrc_set_lock.lock();
    set<uint32_t>::iterator i = ssrc_set.find(ssrc);
    if(i != ssrc_set.end())
    {
        ssrc_set_lock.unlock();
        return;
    }
    _realtime_media->register_media_source( get_session(), ssrc, callId, this );
    ssrc_set.insert(ssrc);
    ssrc_set_lock.unlock();
}


Realtime_Media_Stream_Receiver_Participant::Realtime_Media_Stream_Receiver_Participant(SRef <Sdp_HeaderM *> header, int i, std::string callide,
                                                                                       SRef<Realtime_Media *> m, SRef<Session*> s,
                                                                                       SRef<Rtp_Receiver* > one,  SRef<Rtp_Receiver* > two,
                                                                                       SRef <Sip_Dialog*> voipForwarder, int mLineForwarder,
                                                                                       IRequest_Video_Keyframe *keyframeRequestCallback)
    : Realtime_Media_Stream_Receiver(callide, m, s, one , two, keyframeRequestCallback)
{
    headerM = header;
    index = i;
    media_line_forwarder = mLineForwarder;
    project_on_my_display =  false;
}

void Realtime_Media_Stream_Receiver_Participant::handle_rtp_packet( const SRef<SRtp_Packet *> & packet, std::string callId, SRef<IPAddress *> from )
{
    uint32_t packetSsrc;
    uint16_t seq_no;

    //if packet is null, we had a read timeout from the rtpReceiver
    if( !packet )
    {
        return;
    }

    packetSsrc = packet->get_header().get_ssrc();
    seq_no = packet->get_header().get_seq_no();

#ifdef ZRTP_SUPPORT
    SRef<Crypto_Context *> pcc = get_crypto_context(packetSsrc, seq_no);

    // If empty crypto context for this SSRC but we are already in Secure
    // state then create a real CryptoContext for this SSRC. Assumption:
    // every SSRC stream sent via this connection is secured _and_ uses
    // the same crypto parameters.
    if (zrtp_bridge && zrtp_bridge->getSsrcSender() != 0 &&
            zrtp_bridge->isSecureState() &&
            (pcc->getEalg() == MIKEY_SRTP_EALG_NULL)) {
        pcc = zrtp_bridge->newCryptoContextForRecvSSRC(packetSsrc, 0, seq_no, 0L);
    }
    if( packet->unprotect(pcc)) { // Authentication or replay protection failed
        return;
    }
    if (zrtp_bridge && packet->get_header().get_extension() &&
            (zrtp_bridge->process_packet(packet) == 0)) {
        return;
    }
#else
    if( packet->unprotect( get_crypto_context( packetSsrc, seq_no ) ))
    {
        // Authentication or replay protection failed
        //cerr<<"=====================================> athentication failed \n";
        return;
    }

#endif
}

Realtime_Media_Stream_Sender_Participant::Realtime_Media_Stream_Sender_Participant(SRef <Sdp_HeaderM *> header, int i , std::string callide,
                                                                                   SRef<Realtime_Media *> m, SRef<Session*> s, SRef<Rtp_Stream*> rtpStream4,
                                                                                   SRef<Rtp_Stream*> rtpStream6, IStreams_Player_Report_Timestamps *_streamsPlayer)
    : Realtime_Media_Stream_Sender( callide, m , s, rtpStream4, rtpStream6, _streamsPlayer)
{
    headerM = header;
    index = i;
}
#endif
