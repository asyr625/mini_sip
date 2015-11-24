#ifndef RTP_RECEIVER_H
#define RTP_RECEIVER_H

#include "sobject.h"
#include "mutex.h"
#include "thread.h"

#include "ip_provider.h"
#include "rtp_stream.h"

class Realtime_Media_Stream_Receiver;
class Crypto_Context;
class Session;

class Rtp_Receiver : public Runnable
{
public:
    Rtp_Receiver(SRef<Ip_Provider *> ipProvider, std::string callId, IStreams_Player_Report_Timestamps *_streamsPlayer, Session* session);

    void register_realtime_media_stream( SRef<Realtime_Media_Stream_Receiver *> realtimeMediaStream );

    void unregister_realtime_media_stream( SRef<Realtime_Media_Stream_Receiver *> realtimeMediaStream);

    virtual void run();

    void stop();

    void join();

    uint16_t get_port();
    uint16_t get_rtcp_port();

    ~Rtp_Receiver();

    virtual std::string get_mem_object_type() const {return "RtpReceiver";}

    SRef<Rtp_Stream*> get_rtp_stream();

private:
    SRef<Rtp_Stream*> rtp_stream;
    uint16_t external_port;
    uint16_t external_rtcp_port;
    bool kill;

    std::list< SRef<Realtime_Media_Stream_Receiver *> > realtime_media_streams;

    Mutex realtime_media_streams_lock;

    Thread * thread;

    std::string call_id;
};

#endif // RTP_RECEIVER_H
