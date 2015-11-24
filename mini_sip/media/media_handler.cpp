#include "media_handler.h"

#include <signal.h>
#include <string.h>
#include <sstream>
using namespace std;

#include "sdp_packet.h"
#include "key_agreement.h"

#include "sip_configuration.h"
#include "ip_provider.h"
#include "codec.h"
#include "session.h"
#include "media_stream.h"

#include "zrtp_host_bridge_minisip.h"

#include "media.h"
#include "reliable_media_server.h"
#include "rtp_receiver.h"
#include "media_command_string.h"
#include "udp_socket.h"
#include "sound_io.h"
#include "sound_device.h"

#include "call_recorder.h"
#include "session_registry.h"
#include "streams_player.h"

#ifdef _WIN32_WCE
#	include "minisip_wce_extra_includes.h"
#endif

#define STREAMS_PLAYER_MAX_DELAY_ms 200
#define STREAMS_PLAYER_BUFFER_OVERFLOW_UNDERFLOW_MODIFIER_ms 10
#define STREAMS_PLAYER_SYNCHRONIZATION_TOLERATION_ms 10



Media_Handler::Media_Handler( Mini_Sip* _minisip,  SRef<Sip_Configuration *> conf, SRef<Ip_Provider *> ipp, SRef<Ip_Provider *> ip6Provider )
    : minisip(_minisip), ip6_provider(ip6Provider),
      streams_player(new Streams_Player(STREAMS_PLAYER_MAX_DELAY_ms, STREAMS_PLAYER_BUFFER_OVERFLOW_UNDERFLOW_MODIFIER_ms, STREAMS_PLAYER_SYNCHRONIZATION_TOLERATION_ms))
{
    this->ip_provider = ipp;
    this->config = conf;
    init();
}

Media_Handler::~Media_Handler()
{
}

void Media_Handler::init()
{
    media.clear();

    Media_Registry *registry = *Media_Registry::get_instance();
    Media_Registry::const_iterator i, last = registry->end();

    for( i = registry->begin(); i != last; i++ )
    {
        SPlugin *plugin = **i;
        Media_Plugin *mediaPlugin = dynamic_cast<Media_Plugin*>(plugin);
        if( mediaPlugin )
        {
            SRef<Media *> m = mediaPlugin->create_media( minisip, config, *streams_player );

            if( m )
                register_media( m );

            if(!audio_media)
            {
                Audio_Media *audio = dynamic_cast<Audio_Media *>(*m);
                if(audio)
                    audio_media = audio;
            }
        }
    }

    Session::registry = this;
    ringtone_file = config->_ringtone;
}

SRef<Session *> Media_Handler::create_session( SRef<Sip_Identity*> id, std::string callId )
{
    std::list< SRef<Media *> >::iterator i;
    SRef<Session *> session;
    SRef<Rtp_Receiver *> rtpReceiver = NULL;
    SRef<Rtp_Receiver *> rtp6Receiver;
    string contactIp;
    string contactIp6;

#ifdef ZRTP_SUPPORT
    SRef<ZrtpHostBridgeMinisip *> zhb = NULL;
#endif


    if( ip_provider )
        contactIp = ip_provider->get_external_ip();

    if( ip6_provider )
        contactIp6 = ip6_provider->get_external_ip();

    session = new Session(minisip, callId, contactIp, id, *streams_player, contactIp6 );

    /* passing the supported media list to the session so as to reply to mutiple m lines */

    session->set_media_list(media);
    session->set_ip_provider(ip_provider);
    session->set_ip6_provider(ip6_provider);

    //int index_media_line = 0;

    for( i = media.begin(); i != media.end(); i++ )
    {
        //	index_media_line++;
        SRef<Media *> m = *i;
        SRef<Realtime_Media*> rtm = dynamic_cast<Realtime_Media*>(*m);
        SRef<Reliable_Media*> relm = dynamic_cast<Reliable_Media*>(*m);

        if (relm)
        {
            session->add_reliable_media_session( relm->create_media_stream(callId) );
        }

        if( rtm && rtm->receive )
        {
            if( ip_provider )
                rtpReceiver = new Rtp_Receiver( ip_provider, callId, *streams_player, *session );

            if( ip6_provider )
                rtp6Receiver = new Rtp_Receiver( ip6_provider, callId, *streams_player, *session );

            SRef<Realtime_Media_Stream_Receiver *> rStream;
            rStream = new Realtime_Media_Stream_Receiver( callId, rtm, session, rtpReceiver, rtp6Receiver, *session);
            /* no medi line has assinged to that streamer*/
            rStream->set_peer_uri(session->get_name());
            rStream->set_position_media_line(0);
            session->add_realtime_media_stream_receiver( rStream );

            /* FIXME: The call recorder makes the audio output sound bad. Most likely,
 * it causes incoming audio to be put into the jitter buffer twice which
 * makes it overflow. Not sure why. FIXME.

            // Need to dereference SRef:s, Can't compare SRef:s
            // of different types
            if( *rtm == *audioMedia ) {
                CallRecorder * cr;
                cr = new CallRecorder( audioMedia, rtpReceiver, ipProvider );
                session->callRecorder = cr;
            }
*/

#ifdef ZRTP_SUPPORT
            if(/*securityConfig.use_zrtp*/ id->use_zrtp) {
#ifdef DEBUG_OUTPUT
                cerr << "Media_Handler::create_session: enabling ZRTP for receiver" << callId << endl;
#endif

                zhb = new ZrtpHostBridgeMinisip(callId, *message_router_callback);
                zhb->setReceiver(rStream);
                rStream->setZrtpHostBridge(zhb);
            }
#endif
        }

        if( rtm && rtm->send ){
            if( !rtpReceiver && !ip_provider.is_null() ){
                rtpReceiver = new Rtp_Receiver( ip_provider, callId, *streams_player, *session );
            }

            if( !rtp6Receiver && !ip6_provider.is_null() ){
                rtp6Receiver = new Rtp_Receiver( ip6_provider, callId, *streams_player, *session );
            }

            SRef<Rtp_Stream*> rtpStream4;
            SRef<Rtp_Stream*> rtpStream6;

            if( rtpReceiver )
                rtpStream4 = rtpReceiver->get_rtp_stream();
            if( rtp6Receiver )
                rtpStream6 = rtp6Receiver->get_rtp_stream();

            //////////////////////////////
            if( rtm->get_sdp_media_type().compare("video")  == 0 )
            {
                session->set_video_rtp_streams(rtpStream4, rtpStream6);
                if(config->_video_stream_ipv6_dscp != 0 && rtpStream6)
                {
                    SRef<UDP_Socket*> rtpSocket =  rtpStream6->get_rtp_socket();
                    if(rtpSocket) {
                        rtpSocket->set_ipv6_dscp(config->_video_stream_ipv6_dscp);
                    }
                }
            }
            else
            {
                if(config->_video_stream_ipv6_dscp != 0 && rtpStream6)
                {
                    SRef<UDP_Socket*> rtpSocket =  rtpStream6->get_rtp_socket();
                    if(rtpSocket) {
                        rtpSocket->set_ipv6_dscp(config->_audio_stream_ipv6_dscp);
                    }
                }
            }


            SRef<Realtime_Media_Stream_Sender *> sStream;
            //sStream = new Realtime_Media_Stream_Sender( callId, rtm, sock, sock6 );
            sStream = new Realtime_Media_Stream_Sender( callId, rtm, session, rtpStream4, rtpStream6, *streams_player );
            /* no medi line has assinged to that streamer*/
            sStream->set_position_media_line(0);
            session->add_realtime_media_stream_sender( sStream );
#ifdef ZRTP_SUPPORT
            if(/*securityConfig.use_zrtp*/ id->use_zrtp) {
#ifdef DEBUG_OUTPUT
                cerr << "Media_Handler::create_session: enabling ZRTP for sender: " << callId << endl;
#endif
                if (!zhb) {
                    zhb = new ZrtpHostBridgeMinisip(callId, *message_router_callback);
                }
                zhb->setSender(sStream);
                sStream->set_zrtp_host_bridge(zhb);
            }
#endif
        }
    }

    //set the audio settings for this session ...
    session->mute_senders( true );
    session->silence_sources( false );

    add_session(session);
    return session;
}

void Media_Handler::add_session ( SRef<Session *> session )
{
    this->session_list.push_back(session);
}

void Media_Handler::delete_session ( std::string callId )
{
    std::list< SRef <Session *> > :: iterator i;
    for ( i = this->session_list.begin() ; i != this->session_list.end(); i++)
    {
        if ( callId.compare((*i)->get_call_id() ) == 0 )
        {
            session_list.erase(i);
            return;
        }
    }
    my_err <<"ERROR: could not delete session with id "<<callId<<". (not found)"<<endl;
}

void Media_Handler::parse_screen_command(char * buf, char **args)
{
    while (*buf != 0 )
    {
        /* Strip whitespace.  Use nulls, so that the previous argument is terminated automatically. */
        while ((*buf == ' ') || (*buf == '\t'))
            *buf++ = 0;

        /* Save the argument. */
        *args++ = buf;

        /* Skip over the argument. */
        while ((*buf != 0) && (*buf != ' ') && (*buf != '\t'))
            buf++;
    }

    *args = NULL;
}

SRef<Session *> Media_Handler::get_session (std::string callId)
{
    std::list< SRef <Session *> > :: iterator i;
    for ( i = this->session_list.begin() ; i != this->session_list.end(); i++)
    {
        if ( callId.compare((*i)->get_call_id() ) == 0 )
        {
            return (*i);
        }
    }
    return NULL;
}

void Media_Handler::register_media( SRef<Media *> media )
{
    this->media.push_back( media );
}

void Media_Handler::handle_command(std::string subsystem, const Command_String & command )
{
    assert(subsystem == "media");

    if (command.get_op()=="create_session")
    {
        string callid = command.get_destination_id();
        string sipIdentityId = command.get_param();
        my_assert(callid != "" && sipIdentityId!="" );
        SRef<Sip_Identity*> id;
        std::list< SRef<Sip_Identity*> >::iterator i;
        for (i=config->_identities.begin(); i!=config->_identities.end(); i++)
        {
            if ( (*i)->get_id() == sipIdentityId)
                id = *i;
        }
        my_assert(id);
        create_session(id, callid);
        return;
    }

    if (command.get_op()=="delete_session")
    {
        string callid = command.get_destination_id();
        my_assert(callid != "");
        SRef<Session*> s = get_session(callid);
        if (s)
        {
            s->stop();
            s->unregister();
            delete_session(callid);
        }
        return;
    }


    if( command.get_op() == Media_Command_String::start_ringing )
    {
        // 		cerr << "MediaHandler::handleCmd - start ringing" << endl;
        if( audio_media && ringtone_file != "" ){
            audio_media->start_ringing( ringtone_file );
        }
        return;
    }

    if( command.get_op() == Media_Command_String::stop_ringing )
    {
        // 		cerr << "MediaHandler::handleCmd - stop ringing" << endl;
        if( audio_media ){
            audio_media->stop_ringing();
        }
        return;
    }

    if( command.get_op() == Media_Command_String::session_debug )
    {
#ifdef DEBUG_OUTPUT
        cerr << get_debug_string() << endl;
#endif
        return;
    }

    if( command.get_op() == Media_Command_String::send_dtmf)
    {
        SRef<Session *> session = Session::registry->get_session( command.get_destination_id() );
        if( session )
        {
            string tmp = command.get_param();
            if (tmp.length()==1){
                uint8_t c = tmp[0];
                session->send_dtmf( c );
            }else{
                my_err("media/dtmf") << "Error: DTMF format error. Ignored."<<endl;
            }
        }
        return;
    }

    if( command.get_op() == Media_Command_String::set_session_sound_settings )
    {
        bool turnOn;
#ifdef DEBUG_OUTPUT
        cerr << "MediaHandler::handleCmd: received set session sound settings"
             << endl << "     " << command.get_string()  << endl;
#endif
        if( command.get_param2() == "ON" ) turnOn = true;
        else turnOn = false;
        set_session_sound_settings( command.get_destination_id(), command.get_param(), turnOn );
        return;
    }

    if( command.get_op() == Media_Command_String::reload )
    {
        init();
        return;
    }

    if( command.get_op() == "call_recorder_start_stop" )
    {
#ifdef DEBUG_OUTPUT
        cerr << "MediaHandler::handleCmd: call_recorder_start_stop" << endl
             << command.get_string() << endl;
#endif
        bool start = (command.get_param() == "START" );
        session_call_recorder_start( command.get_destination_id(), start );
        return;
    }

    SRef <Session * > sessionTmp;
    if( command.get_op() == Media_Command_String::start_camera )
    {
        string tmp = command.get_param();
        sessionTmp = get_session(tmp);
        //		int DestPort =  sessionTmp->get_destination_port();
        string DestIp = sessionTmp ->get_destination_ip ()->get_string();
        //		cout << "\n\n\n.........................................\n\n\nFINALLY destination Ip and destination Port .... " << DestIp << ":"<<DestPort<<endl;

        SRef<Media_Registry*> registry = Media_Registry::get_instance();
        Media_Registry::const_iterator i;
        Media_Registry::const_iterator last = registry->end();


        SRef<Realtime_Media*> videoMedia ;
        // this is bad i should create a VideoPlugin only or just a videoMedia
        for( i = registry->begin(); i != last; i++ )
        {
            SRef<SPlugin *> plugin = *i;
            SRef<Media_Plugin *> videoPlugin = dynamic_cast<Media_Plugin*>( *plugin );
            if( (videoPlugin->get_name()).compare("video") == 0  )
            {
                cout << "............ENTER FOR VIDEO VideoPlugin get_name ..........."<<videoPlugin->get_name()<<endl;
                //config->videoDevice =  "/dev/video0";
                SRef<Media*> ms = videoPlugin ->create_media2stream( minisip, config, *streams_player );
                //			cout << "...........testing for video........." << ms -> getSdpMediaType()<<endl;
                videoMedia =  dynamic_cast<Realtime_Media*> (* ms);
            }
        }

        sstream = new Realtime_Media_Stream_Sender( tmp,videoMedia, sessionTmp, sessionTmp->get_video_rtp_stream4( ), sessionTmp->get_video_rtp_stream6( ), *streams_player);

        (*sstream)->set_port( sessionTmp->get_destination_port() );
        (*sstream)->set_remote_address( sessionTmp->get_destination_ip()  );

        sessionTmp->add_realtime_media_stream_sender( sstream );

        //		  sStream->setSelectedCodecHacked(videoMedia); //FIXME: make sure second stream is working even without this

        (*sstream)->start();

        return;
    }
    if( command.get_op() == Media_Command_String::stop_camera )
    {
        (*sstream)->stop();
        // sessionTmp ->removeRealtime_Media_Stream_Sender( sStream );
        return;
    }

    //int screen_pid;
    static pid_t pid;

    if( command.get_op() == Media_Command_String::start_screen )
    {
        string tmp = command.get_param();
        SRef <Session * > sessionTmp = get_session(tmp);
        int DestPort =  sessionTmp->get_destination_port();
        string DestIp = sessionTmp ->get_destination_ip ()->get_string();
        cout << "FINALLY destination Ip and destination Port .... " << DestIp << ":"<<DestPort<<endl;
        cout << " display frame rate "<< config->_display_frame_rate <<" display frame size " <<  config->_display_frame_rate <<endl;


        /******************************* SCREEN STREAM WITH FFMPEG LIBRARY ******************************/
        //int status;
        FILE *fpipe;
        const char *command="echo $DISPLAY";
        char display_variable[500];
        char *args[256];
        std::string destPort_str;
        std::stringstream destPort_stream;
        char *ffmpeg_command_array;
        std::string ffmpeg_command;

        if ( !(fpipe = (FILE*)popen(command,"r")) ) {
            // If fpipe is NULL
            perror("Problems with pipe");
            exit(1);
        }

        while ( fgets( display_variable, sizeof display_variable, fpipe))
        {
            printf("%s", display_variable);
        }
        pclose(fpipe);

        char* p = strchr(display_variable,'\n');
        if (p) {
            *p = '\0';
        }

        destPort_stream << DestPort;
        destPort_str = destPort_stream.str();

        ffmpeg_command = "ffmpeg -f x11grab -s " + config->_display_frame_size + " -r " + config->_display_frame_rate + " -i " + display_variable + " -vcodec libx264 -vpre lossless_fast -f rtp rtp://" + DestIp + ":" + destPort_str;
        cout << ffmpeg_command << endl;


        /*if (ffmpeg_command == NULL) {
                    printf("\n");
                    exit(0);
            } */
        ffmpeg_command_array = new char[ffmpeg_command.size()+1];
        ffmpeg_command_array[ffmpeg_command.size()]=0;
        memcpy(ffmpeg_command_array,ffmpeg_command.c_str(),ffmpeg_command.size());

        /* Split the string into arguments. */
        parse_screen_command(ffmpeg_command_array, args);

        /**************** Execute the command. ***********/

        /* Get a child process. */
        if ((pid = fork()) < 0)
        {
            perror("fork");
            exit(1);

        }

        /* The child executes the code inside the if.*/
        if (pid == 0)
        {
            //screen_pid = getpid();
            cout << " ********************  The process id of the screen stream is: " << pid << " ************************" << endl;

            execvp(*args, args);

            //perror(*args);
            //exit(1);
        }
        else if(pid > 0)
        {
            cerr<< "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n";
            sessionTmp->set_pid(pid);

            cout << "************************  I am the parent process streaming! ******************** My process id is: " << getpid() << " The process id of my child is: " <<sessionTmp->get_pid()<< endl;
            //		get_session(tmp)->pid = pid; // charis
            //screen_pid = getpid();
            cout << " ******************** 2  The process id of the screen stream is: " << pid << " ************************" << endl;
        }


        //execute_screen_command(args);
        /************************************************************************************************/
        return;
    }
    if( command.get_op() == Media_Command_String::stop_screen ){
        //		cout << "************************------- Stop screen: ******************** My process id is: " << getpid() << " The process id of my child is: " << pid << endl;
        if (pid != 0 ) kill( pid, SIGKILL );
        return;
    }

    my_dbg <<"WARNING: MediaHandler::handleCommand: command not handled: "<<command.get_string()<<endl;
}

Command_String Media_Handler::handle_command_resp(std::string subsystem, const Command_String& command)
{
    assert(subsystem=="media");

    if (command.get_op()=="get_sdp_offer")
    {
        string callid = command.get_destination_id();
        my_assert( callid != "" );
        SRef<Session*> session = get_session(callid);
        bool anat = ("yes" == command["anat"]);
        string peerUri = command.get_param();
        SRef<Sdp_Packet *> sdp = session->get_sdp_offer( peerUri, anat );
        string sdpstr = sdp->get_string();
        return Command_String( command.get_destination_id(), sdpstr );
    }

    if (command.get_op()=="set_sdp_answer")
    {
        string callid = command.get_destination_id();
        my_assert( callid != "" );
        SRef<Session*> session = get_session(callid);
        string sdpstr = command.get_param();
        SRef<Sdp_Packet*> sdp = new Sdp_Packet(sdpstr);
        string peerUri = command.get_param2();
        bool ret = session->set_sdp_answer( sdp, peerUri );
        return Command_String( command.get_destination_id(), ret?"ok":"fail" );
    }


    if (command.get_op()=="set_sdp_offer")
    {
        string callid = command.get_destination_id();
        my_assert( callid != "" );
        SRef<Session*> session = get_session(callid);
        string sdpstr = command.get_param();
        SRef<Sdp_Packet*> sdp = new Sdp_Packet(sdpstr);
        string peerUri = command.get_param2();
        bool ret = session->set_sdp_offer( sdp, peerUri );
        return Command_String( command.get_destination_id(), ret?"ok":"fail" );
    }

    if (command.get_op()=="get_sdp_answer")
    {
        string callid = command.get_destination_id();
        my_assert( callid != "" );
        SRef<Session*> session = get_session(callid);
        SRef<Sdp_Packet *> sdp = session->get_sdp_answer();
        string sdpstr = sdp->get_string();
        return Command_String( command.get_destination_id(), sdpstr );
    }


    my_dbg << "WARNING: MediaHandler::handleCommandResp: command not handled:" << command.get_string() << endl;
    return Command_String("","not_handled");
}

std::string Media_Handler::get_ext_ip()
{
    return ip_provider->get_external_ip();
}

void Media_Handler::set_session_sound_settings( std::string callid, std::string side, bool turnOn )
{
    std::list<SRef<Session *> >::iterator iSession;

    //what to do with received audio
    if( side == "receivers" )
    {
        _session_lock.lock();
        for( iSession = _sessions.begin(); iSession != _sessions.end(); iSession++ )
        {
            if( (*iSession)->get_call_id() == callid ){
                //the meaning of turnOn is the opposite of the Session:: functions ... silence/mute
                (*iSession)->silence_sources( ! turnOn );
            }
        }
        _session_lock.unlock();
    } else if ( side == "senders" ) //what to do with audio to be sent over the net
    {
        //set the sender ON as requested ...
        _session_lock.lock();
        for( iSession = _sessions.begin(); iSession != _sessions.end(); iSession++ )
        {
            if( (*iSession)->get_call_id() == callid ){
                //the meaning of turnOn is the opposite of the Session:: functions ... silence/mute
                (*iSession)->mute_senders( !turnOn );

            }
        }
        _session_lock.unlock();
    } else {
        cerr << "Media_Handler::set_session_sound_settings - not understood" << endl;
        return;
    }
}

void Media_Handler::session_call_recorder_start( std::string callid, bool start )
{
    Call_Recorder * cr;
    std::list<SRef<Session *> >::iterator iSession;

    _session_lock.lock();
    for( iSession = _sessions.begin(); iSession != _sessions.end(); iSession++ )
    {
        if( (*iSession)->get_call_id() == callid )
        {
            cr = dynamic_cast<Call_Recorder *>( *((*iSession)->call_recorder) );
            if( cr ) {
                cr->set_allow_start( start );
            }
        }
    }
    _session_lock.unlock();
}

SRef<Media*> Media_Handler::get_media(std::string sdpType)
{
    std::list<SRef<Media*> >::iterator i;
    for (i = media.begin(); i!=media.end(); i++)
    {
        if ((*i)->get_sdp_media_type() == sdpType)
            return *i;
    }
    return NULL;
}

#ifdef DEBUG_OUTPUT
std::string Media_Handler::get_debug_string()
{
    std::string ret;
    ret = get_mem_object_type() + ": Debug Info\n";
    for( std::list<SRef<Session *> >::iterator it = _sessions.begin();
         it != _sessions.end(); it++ )
    {
        ret += "** Session : \n" + (*it)->get_debug_string() + "\n";
    }
    return ret;
}
#endif
