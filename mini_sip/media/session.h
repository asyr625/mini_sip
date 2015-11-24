#ifndef SESSION_H
#define SESSION_H

#include "sobject.h"
#include "thread.h"
#include "cond_var.h"
#include "timeout_provider.h"

#include "media_stream.h"

#include "sip_dialog.h"
#include "sdp_packet.h"
#include "rtp_stream.h"
#include "dtmf_sender.h"
#include "ip_provider.h"
#include "sip_configuration.h"
#include "irequest_video_keyframe.h"
#include "session_registry.h"


#define BASELINE_PROFILE 0
#define MAIN_PROFILE 1

#define KEYFRAME_REQUEST_MIN_INTERVAL_ms 1000

class Mikey;
class Key_Agreement;
class Realtime_Media_Stream_Sender;

class Session : public virtual Runnable, public IRequest_Video_Keyframe
{
public:
    static Session_Registry* registry;
    static SRef<Key_Agreement*> precompute_ka;

    Session(Mini_Sip* minisip, std::string call_id, std::string localIp, SRef<Sip_Identity*> ident, IStreams_Player_Report_Timestamps *_streams_player, std::string localIp6 = "" );

    ~Session();

    void unregister();

    void start();

    void refresh();
    void stop();

    SRef <Sdp_Packet *> get_last_sdp_answer (){ return last_sdp_answer;}
    void set_last_sdp_answer ( SRef <Sdp_Packet *> pack ){ last_sdp_answer = pack ; }
    SRef <Sdp_Packet *> get_last_sdp_answer_mcu (){ return last_sdp_answer;}
    void set_last_sdp_answer_mcu ( SRef <Sdp_Packet *> pack ){ last_sdp_answer = pack ; }

    void  set_sdp_offer_200OK(SRef < Sdp_Packet *> pckt );

    SRef <Sdp_Packet *> get_last_sdp_packet();
    void set_last_sdp_packet (SRef <Sdp_Packet *> p );

    SRef<Sdp_Packet *> get_sdp_offer(const std::string &peerUri = "", bool anatSupported = false );

    SRef<Sdp_Packet *> get_sdp_answer();
    SRef<Sdp_Packet *> get_sdp_answer(SRef < Sdp_Packet * > sdpPack);
    SRef<Sdp_Packet *> get_sdp_answer_200Ok();

    bool set_sdp_answer( SRef<Sdp_Packet *> answer, std::string peer_uri );

    bool set_sdp_offer (SRef<Sdp_Packet *> offer, std::string peerUri );

    void add_reliable_media_session(SRef<Reliable_Media_Stream*> rsess);

    void add_realtime_media_stream_receiver(SRef<Realtime_Media_Stream_Receiver *> realtimeMediaStream );

    void add_realtime_media_stream_receiver_top(SRef<Realtime_Media_Stream_Receiver *> realtimeMediaStream );

    void add_realtime_media_stream_sender(SRef<Realtime_Media_Stream_Sender*> realtimeMediaStream );

    void remove_realtime_media_stream_sender( SRef<Realtime_Media_Stream_Sender*> realtimeMediaStream );

    void add_realtime_media_stream_sender_top(SRef<Realtime_Media_Stream_Sender*> realtimeMediaStream );

    std::string get_error_string();

    uint16_t get_error_code();

    bool is_secure();

    virtual std::string get_mem_object_type() const {return "Session";}

    std::string get_call_id();

    void set_call_id( const std::string call_id );

    friend class Dtmf_Sender;

    void send_dtmf( uint8_t symbol );

    void mute_senders (bool mute);

    bool muted_senders;

    void silence_sources ( bool silence );

    bool silenced_sources;

    virtual std::string get_debug_string();

    void clear_realtime_media_stream_receivers();

    std::list< SRef<Realtime_Media_Stream_Receiver *> > get_realtime_media_stream_receivers();

    SRef<Realtime_Media_Stream_Receiver *> get_realtime_media_stream_receiver(uint32_t ssrc);

    SRef<Realtime_Media_Stream_Sender*> get_realtime_media_stream_sender(uint32_t ssrc);

    std::list< SRef<Realtime_Media_Stream_Sender *> > get_realtime_media_stream_senders();

    const std::string &get_peer_uri() const;
    SRef<Sip_Identity*> get_own_identity();

    SRef<SObject *> call_recorder;

    void set_media_list (  std::list< SRef<Media *> > mediaL ) { media = mediaL;}
    std::list< SRef<Media *> >  get_media_list () { return media; }

    SRef<Media*> get_media_of_type(std::string);

    void set_ip_provider ( SRef<Ip_Provider *> ipProvidertmp ){ ip_provider = ipProvidertmp;}
    SRef<Ip_Provider *> get_ip4_provider (){return ip_provider;}

    void set_ip6_provider ( SRef<Ip_Provider *> ipProvidertmp ){ ip6_provider = ipProvidertmp;}
    SRef<Ip_Provider *> get_ip6_provider (){return ip6_provider;}

    SRef<Ip_Provider *> get_actual_ip_provider() { return ip6_provider->get_external_ip().empty() ? ip_provider : ip6_provider; }
    std::string get_local_ip() { return local_ip6_string.empty() ? local_ip_string : local_ip6_string; }

    pid_t  get_pid();

    void  set_pid(  pid_t p);

    void set_destination_port ( int port){ destination_port = port;}
    void set_destination_ip (  SRef<IPAddress *> s ) { destination_ip = s ;}

    int get_destination_port ( ){ return destination_port;}
    SRef <IPAddress *> get_destination_ip ( ) { return destination_ip;}

    void set_video_rtp_streams(SRef<Rtp_Stream*> _rtpStream4, SRef<Rtp_Stream*> _rtpStream6)
    {
        video_rtp4 = _rtpStream4;
        video_rtp6 = _rtpStream6;
    }
    SRef<Rtp_Stream*> get_video_rtp_stream4(){return video_rtp4;}
    SRef<Rtp_Stream*> get_video_rtp_stream6(){return video_rtp6;}

    int get_profile();
    pid_t pid ;
    SRef<Sdp_Packet *> empty_sdp();

    void add_realtime_media_stream_receiver_participant(SRef <Sdp_HeaderM *> headerM , int index, SRef< Sip_Dialog *> f , int m  );
    void add_realtime_media_stream_receiver_participant_top(SRef <Sdp_HeaderM *> headerM , int index, SRef< Sip_Dialog *> f , int m );

    void set_zero_participants(int index);

    int  get_sdp_version_number() { return SdpVersionNumber; }
    void set_sdp_version_number(int i) { SdpVersionNumber = i; }
    int SdpVersionNumber;

    SRef< Sdp_HeaderM* > set_direction_attribute(SRef < Sdp_HeaderM *> offerHdr , SRef < Sdp_HeaderM *> ansHdr ,  SRef < Realtime_Media_Stream_Receiver * > recv, int index );

    bool check_mikey_in_offer( SRef<Sdp_Packet *> offer, std :: string peer_uri );
    bool check_mikey_in_answer(SRef <Sdp_Packet *> answer , std :: string peerUri);

    SRef<Realtime_Media_Stream_Sender *>  get_sender_at_media_line(int line );

    SRef <Sdp_HeaderM* > set_receiver_connection_data (SRef <Sdp_HeaderM* > headerM , int mediaLine);
    void set_sender_destination_param ( SRef <Sdp_HeaderM* > headerM , int mediaLine);
    int32_t  get_receiver_port (int mediaLine );

    void storing_media_line (SRef <Sdp_HeaderM* > headerM , int mediaLine);

    SRef<Sdp_Packet *> get_sdp_packet_empty();
    void stop_streamers_at_line(int mediaLine);

    std:: string get_name() { return peer_uri;}
    SRef<Realtime_Media_Stream_Sender *>  create_real_time_media_streams ( int index , std :: string mediaType,std:: string direction );

    void clear_streamers();
    bool is_active() { return started; }
    void video_keyframe_request_arrived();

    void set_keyframe_request_callback(IRequest_Video_Keyframe *_keyframe_request_callback);

    virtual void try_requesting_video_keyframe();

    virtual void run();

    Mini_Sip* get_mini_sip();
    SRef<Message_Router*> get_message_router();
protected:
    Mini_Sip* minisip;

    bool add_realtime_media_to_offer(SRef<Sdp_Packet*> result, const std::string &peer_uri, bool anatSupported, std::string transport);
    bool add_reliable_media_to_offer(SRef<Sdp_Packet*> result, const std::string &peer_uri, bool anatSupported);


    bool started;
    void add_streams();

    std::string peer_uri;
    int destination_port;
    SRef<IPAddress *> destination_ip;
    SRef<Rtp_Stream*> video_rtp4;
    SRef<Rtp_Stream*> video_rtp6;

    SRef<Realtime_Media_Stream_Receiver *> match_format( SRef<Sdp_HeaderM *> m,
                                                     uint32_t iFormat, SRef<IPAddress *> &remoteAddress, int index );

    std::list<SRef<Reliable_Media_Stream*> > reliable_media_sessions;

    typedef std::list< SRef<Realtime_Media_Stream_Sender *> > Realtime_Media_Stream_Senders;
    std::list< SRef<Realtime_Media_Stream_Receiver *> > realtime_media_stream_receivers;
    std::list< SRef<Realtime_Media_Stream_Sender *> > realtime_media_stream_senders;
    Mutex realtime_media_stream_senders_lock;

    SRef<Mikey *> mikey;
    std::string local_ip_string;
    std::string local_ip6_string;
    SRef<Sdp_Packet *> sdp_answer;

    SRef<Sdp_Packet *> last_sdp_answer;

    SRef<Sdp_Packet *> last_sdp_packet;

    SRef<Sdp_Packet *> last_sdp_answer_mcu;


    std::string error_string;
    uint16_t error_code;
    SRef<Sip_Identity*> identity;

    int ka_type;


    std::string call_id;

    SRef<Timeout_Provider<Dtmf_Event *, SRef<Dtmf_Sender *> > *> dtmf_to_provider;
    int8_t profile_saved;

    std::list< SRef<Media *> > media;
    SRef<Ip_Provider *> ip_provider;
    SRef<Ip_Provider *> ip6_provider;

    IStreams_Player_Report_Timestamps *streams_playerr;
    IRequest_Video_Keyframe *keyframeRequestCallback;
    int64_t timestampOfLastKeyframeRequest_ms;
    Thread *keyframeRequestThread;
    Mutex keyframeRequestMutex;
    bool keyframeRequestThreadQuit, keyframeRequestCalled;
    Cond_Var keyframeRequestCondVar;
};

#endif // SESSION_H
