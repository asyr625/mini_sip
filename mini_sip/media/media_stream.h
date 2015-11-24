#ifndef MEDIA_STREAM_H
#define MEDIA_STREAM_H
#include <set>

#include "media.h"
#include "media_processor.h"
#include "sobject.h"
#include "reliable_media.h"
#include "realtime_media.h"
#include "rtp_receiver.h"
#include "rtp_stream.h"

#include "sip_dialog.h"

#include "key_agreement.h"

class Realtime_Media;
class IRequest_Video_Keyframe;

class Media_Stream : public SObject
{
public:
    Media_Stream(std::string callId_, SRef<Media*> m, SRef<Session*> s);
    virtual void start() = 0;

    virtual void stop() = 0;

    virtual std::string get_debug_string();

    std::string get_sdp_media_type() const; /* audio, video, application... */
    std::list<std::string> get_sdp_attributes();

    std::string get_call_id() const;

    virtual uint16_t get_port() = 0;
    SRef<Media *> get_media() const;

    SRef<Session*> get_session() const;

    std::string _direction_attribute;
protected:
    std::string _call_id;
    SRef<Media *> _media;
    SRef<Session*> _session;
};


class Reliable_Media_Stream : public Media_Stream
{
public:
    Reliable_Media_Stream(std::string callId, SRef<Reliable_Media*> m, SRef<Session*> s);

    std::string get_transport(){return "TCP";}

    virtual std::string get_media_formats() = 0;

    virtual uint16_t get_port(std::string type) = 0;

protected:
    uint16_t _port;
    std::string _media_formats;
};


class Realtime_Media_Stream : public Media_Stream
{
public:
    virtual std::string get_mem_object_type() const {return "RealtimeMediaStream";}
    bool _disabled;

    virtual bool matches( SRef<Sdp_HeaderM *> m, uint32_t formatIndex );

    void set_key_agreement( SRef<Key_Agreement *> ka );

#ifdef ZRTP_SUPPORT
    virtual void set_zrtp_host_bridge(SRef<Zrtp_Host_Bridge_Minisip *> zsb);
    virtual SRef<Zrtp_Host_Bridge_Minisip *> get_zrtp_host_bridge();
    void set_key_agreement_zrtp(SRef<Crypto_Context *>cx);
#endif

    void flush_crypto_contexts()
    {
        _ka_lock.lock();
        _crypto_contexts.clear();
        _ka_lock.unlock();
    }
    SRef<Realtime_Media *> get_realtime_media();

    virtual void send_rtcp_packet(SRef<Rtcp_Packet*> p) = 0;

protected:
    Realtime_Media_Stream(std::string callId, SRef<Realtime_Media *> m, SRef<Session*> s);

    virtual SRef<Realtime_Media_Pipeline*> get_pipeline(uint32_t ssrc) = 0;

    SRef<Crypto_Context *> get_crypto_context( uint32_t ssrc, uint16_t seq_no );
    SRef<Realtime_Media *> _realtime_media;

    std::map<uint32_t, SRef<Realtime_Media_Pipeline*> > _ssrc_pipeline;
    uint32_t _csb_id;

    std::string _local_payload_type;

    SRef<Crypto_Context *> init_crypto( uint32_t ssrc, uint16_t seq_no );
    SRef<Key_Agreement *> _ka;
    Mutex _ka_lock;
    std::list< SRef<Crypto_Context *> > _crypto_contexts;
#ifdef ZRTP_SUPPORT
    SRef<Zrtp_Host_Bridge_Minisip *> zrtp_bridge;
#endif
};


class Sending_MSS_Reporter
{
public:
    Sending_MSS_Reporter()
        : totalSentPacketSize(0), lastReportedTotalSentPacketSize(0),
          sentPacketCount(0), timestampOfLastSendingThoughputGet_ms(0)
    {
    }
    virtual std::string get_encoder_description() = 0;
    virtual uint64_t get_number_of_sent_packets();
    virtual int get_sent_throughput_kbps();
    virtual float get_sent_video_framerate_fps() = 0;

protected:
    uint64_t totalSentPacketSize, lastReportedTotalSentPacketSize, sentPacketCount;
    int64_t timestampOfLastSendingThoughputGet_ms;
};


class Realtime_Media_Stream_Sender : public Realtime_Media_Stream, public Sending_MSS_Reporter,
        public Media_Pipeline_Output_Handler
{
public:
    Realtime_Media_Stream_Sender(std::string callId, SRef<Realtime_Media *> m, SRef<Session*> s,
                                  SRef<Rtp_Stream*> rtpStream4, SRef<Rtp_Stream*> rtpStream6,
                                  IStreams_Player_Report_Timestamps *_streamsPlayer);

    virtual std::string get_debug_string();

    virtual std::string get_mem_object_type() const { return "RealtimeMediaStreamSender"; }

    SRef<Encoder_Instance*> get_selected_encoder();

    void set_selected_encoder(SRef<Encoder_Instance*> t )
    {
        selected_encoder = t ;
    }

    virtual void start();
    virtual void stop();

    void stop_sending();

    virtual void set_port( uint16_t port );
    virtual uint16_t get_port();

    uint16_t get_local_port();

    void send(byte_t * data, uint32_t length, uint32_t *givenTs, bool marker = false, bool dtmf = false );
    void send( const SRef<Processing_Data*>& data );

#ifdef ZRTP_SUPPORT
    void send_zrtp(unsigned char* data, int length, unsigned char* payload, int payLen);
    uint16_t get_seq_no() { return seq_no; };
#endif

    void send_rtp_packet(const SRef<Rtp_Packet*> & rtpPacket);
    void send_rtp_packet(const SRef<SRtp_Packet*> & rtpPacket, uint16_t seq_no, uint32_t packetSsrc);

    void set_remote_address(SRef<IPAddress *> ra );

    void set_muted( bool mute ) { muted = mute;}

    bool is_muted() { return muted;}

    bool mute_keep_alive( uint32_t max);

    virtual bool matches( SRef<Sdp_HeaderM *> m, uint32_t formatIndex );

    uint32_t get_ssrc();

    void increase_last_ts( ) { last_ts += 160; }
    uint32_t get_last_ts() { return last_ts; }

    int get_position_media_line(){ return position_media_line;}
    void set_position_media_line( int index){ position_media_line = index;}
    bool is_running(){return running;}
    void set_running(bool b) { running = b ;}
    SRef<IPAddress *> get_remote_address() { return remote_address; }

    SRef<IPAddress *> get_local_address();

    bool sendonly;
    int receiver_forwarder_media_line;

    void set_source_id(uint32_t sId) { source_id = sId ;}
    uint32_t get_source_id() { return  source_id;}
    SRef<Sdp_HeaderM *> get_media_header(){return headerM;}
    void set_media_header( SRef <Sdp_HeaderM *> h ){ headerM = h ;}

    virtual std::string get_encoder_description();
    virtual float get_sent_video_framerate_fps();

    virtual void handle_data(const SRef<Processing_Data*>& data );

    void send_rtcp_packet(SRef<Rtcp_Packet*> p);

    void request_codec_intracoded();

protected:
    void send(Rtp_Stream* rtpStream, const SRef<Rtp_Packet *> &packet, const IPAddress &to_addr, const uint16_t &port);

    SRef<Realtime_Media_Pipeline*> get_pipeline(uint32_t ssrc);

    SRef<Sdp_HeaderM *> headerM;
    uint32_t source_id;
    uint32_t ssrc;
    SRef<Rtp_Stream*> rtp_stream4;
    SRef<Rtp_Stream*> rtp_stream6;

    uint16_t remote_port;
    uint16_t seq_no;
    uint32_t last_ts;
    SRef<IPAddress *> remote_address;
    Mutex sender_lock;

    std::string payload_type;
    SRef<Encoder_Instance*> selected_encoder;
    bool muted;
    uint32_t mute_counter;
    bool running;

    int position_media_line;
};

class Realtime_Media_Stream_Receiver : public Realtime_Media_Stream, public Media_Pipeline_Output_Handler
{
public:
    Realtime_Media_Stream_Receiver(std::string callId,  SRef<Realtime_Media *> m, SRef<Session*> s,
                                    SRef<Rtp_Receiver *> rtpReceiver, SRef<Rtp_Receiver *> rtp6Receiver,
                                    IRequest_Video_Keyframe *keyframeRequestCallback);

    virtual std::string get_debug_string();

    virtual void handle_data(const SRef<Processing_Data*>& data );

    virtual std::string get_mem_object_type() const {return "RealtimeMediaStreamReceiver";}

    virtual void start();

    virtual void stop();

    virtual uint16_t get_port();
    virtual uint16_t get_rtcp_port();

    uint16_t get_port( const std::string &addrType );
    uint16_t get_rtcp_port( const std::string &addrType );

    virtual void handle_rtp_packet( const SRef<SRtp_Packet *> & packet, std::string callId, SRef<IPAddress *> from );

    uint32_t get_id();

    std::list<SRef<Codec_Description *> >& get_available_codecs();

    std::set<uint32_t>& get_ssrc_set()
    {
        return ssrc_set;
    }

    void send_rtcp_app_view(unsigned subtype, uint32_t sender_ssrc, int width, int height);

    void send_rtcp_fir(uint32_t ssrc);

#ifdef ZRTP_SUPPORT
    virtual void handle_rtp_packet_ext(SRef<SRtp_Packet *> packet);
#endif

    int get_position_media_line() {return position_media_line; }
    void set_position_media_line( int index) { position_media_line = index; }

    bool is_initiated() { return initiated; }
    bool is_running() { return running; }
    void set_running(bool b) { running = b ;}
    void set_initiated(bool b) { initiated = b ;}
    bool recvonly;

    SRef<Sdp_HeaderM *> get_media_header() { return headerM; }
    void set_media_header( SRef <Sdp_HeaderM *> h ) { headerM = h; }

    void stop_receiving();

    void set_peer_uri ( std :: string peerUri ) { uri = peerUri; }
    std :: string get_peer_uri (){ return uri; }

    void send_rtcp_packet(SRef<Rtcp_Packet*> p);

    SRef<Rtp_Receiver*> get_rtp_receiver(bool ipv6)
    {
        return ipv6 ? rtp6_receiver : rtp_receiver;
    }

protected:
    SRef<Realtime_Media_Pipeline*> get_pipeline(uint32_t ssrc);

    std::list<SRef<Codec_Description *> > codec_list;
    SRef<Rtp_Receiver *> rtp_receiver;
    SRef<Rtp_Receiver *> rtp6_receiver;
    uint32_t id;
    std :: string uri;
    uint16_t external_port;

    SRef<Sdp_HeaderM *> headerM;

    void got_ssrc( uint32_t ssrc, std::string callId );

    std::set<uint32_t> ssrc_set;
    Mutex ssrc_set_lock;

    bool initiated;
    bool running;
    int position_media_line;
};

class Realtime_Media_Stream_Receiver_Participant : public Realtime_Media_Stream_Receiver
{
public:
    Realtime_Media_Stream_Receiver_Participant(SRef <Sdp_HeaderM *> header, int i, std::string callide,
                                               SRef<Realtime_Media *> m, SRef<Session*> session,
                                               SRef<Rtp_Receiver* > one,  SRef<Rtp_Receiver* > two,
                                               SRef <Sip_Dialog*> voipForwarder, int mLineForwarder,
                                               IRequest_Video_Keyframe *keyframeRequestCallback);

    virtual std::string get_debug_string() { return get_mem_object_type(); }
#ifdef ZRTP_SUPPORT
    virtual void handle_rtp_packet_ext(SRef<SRtp_Packet *> packet) { }
#endif

    virtual std::string get_mem_object_type() const {return "RealtimeMediaStreamReceiverParticipant";}

    virtual void handle_rtp_packet( const SRef<SRtp_Packet *> & packet, std::string callId, SRef<IPAddress *> from );

    SRef<Sdp_HeaderM *> get_media_header() { return headerM; }
    void set_media_header( SRef <Sdp_HeaderM *> h ){ headerM = h; }
    int get_position_media_line() { return index; }
    void set_position_media_line(int h ) { index = h ;}
private:
    SRef <Sdp_HeaderM *> headerM;
    int index;
    int media_line_forwarder;
    bool project_on_my_display;
};

class Realtime_Media_Stream_Sender_Participant : public Realtime_Media_Stream_Sender
{
public:
    Realtime_Media_Stream_Sender_Participant(SRef <Sdp_HeaderM *> header, int i , std::string callide,
                                             SRef<Realtime_Media *> m, SRef<Session*> s, SRef<Rtp_Stream*> rtpStream4,
                                             SRef<Rtp_Stream*> rtpStream6, IStreams_Player_Report_Timestamps *_streamsPlayer);

    virtual std::string get_debug_string(){return get_mem_object_type();}

    virtual void start() {}
    virtual void stop() {}

    virtual std::string get_mem_object_type() const {return "RealtimeMediaStreamSenderParticipant";}

    SRef<Sdp_HeaderM *> get_media_header() { return headerM; }
    void set_media_header( SRef <Sdp_HeaderM *> h ){ headerM = h; }
    int get_position_media_line() { return index; }
    void set_position_media_line(int h ) { index = h; }
private:
    SRef <Sdp_HeaderM *> headerM;
    int index;
    int media_line_forwarder;
};

#endif // MEDIA_STREAM_H
