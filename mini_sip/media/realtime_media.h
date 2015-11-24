#ifndef REALTIME_MEDIA_H
#define REALTIME_MEDIA_H

#include "media.h"
#include "mutex.h"
#include "codec/codec.h"
#include "session.h"


class Realtime_Media_Stream_Sender;
class Realtime_Media_Stream_Receiver;
class Processing_Data;

enum Rtp_Jitter_Buffer_Mode
{
    RtpJitterBufferModeOff,
    RtpJitterBufferModeLowJitterConstantDelay,
    RtpJitterBufferModeLowestPossibleDelay
};

#define RTP_JITTER_BUFFER_LENGTH 300 // ms
#define RTP_JITTER_BUFFER_WAKE_UP_INTERVAL 10 // ms
#define RTP_JITTER_BUFFER_MODE RtpJitterBufferModeLowestPossibleDelay

class Receiving_MSS_Reporter //Media Source Statistics
{
public:
    Receiving_MSS_Reporter()
        : totalReceivedPacketSize(0), lastReportedTotalReceivedPacketSize(0),
          timestampOfLastReceivingThroughtputGet_ms(0),receivedPacketCount(0),
          lostPacketCount(0)
    {
    }

    virtual std::string get_decoder_description() = 0;
    virtual uint64_t get_number_of_received_packets() = 0;
    virtual uint64_t get_number_of_missing_packets() = 0;
    virtual int get_received_throughput_kbps();
    virtual float get_received_video_framerate_fps() = 0;

protected:
    uint64_t totalReceivedPacketSize, lastReportedTotalReceivedPacketSize;
    int64_t timestampOfLastReceivingThroughtputGet_ms;
    uint64_t receivedPacketCount, lostPacketCount;
};

class Sending_MSS_Reporter; //

class Realtime_Media : public Media
{
public:
    ~Realtime_Media();
    virtual SRef<Codec_Description*> get_codec( uint8_t payloadType );
    virtual void play_data( const SRef<Rtp_Packet *> & rtpPacket )=0;
    virtual void send_encoded_data( byte_t * data, uint32_t length, int samplerate, const int64_t &pts_us, bool marker/*=false*/ );
    virtual void send_data_to_streams(const SRef<Processing_Data*>& data);
    virtual void register_realtime_media_sender( SRef<Realtime_Media_Stream_Sender *> sender );
    virtual void unregister_realtime_media_sender( SRef<Realtime_Media_Stream_Sender *> sender );
    virtual void register_media_source(const SRef<Session*>& session, uint32_t ssrc, std::string callId, SRef<Realtime_Media_Stream_Receiver*> rtMSR ) = 0;
    virtual void unregister_media_source( uint32_t ssrc) = 0;

    bool receive;
    bool send;

    std::list< SRef<Codec_Description *> >&  get_available_codecs();

    virtual SRef<Decoder_Instance*> create_decoder_instance( const SRef<Realtime_Media_Stream_Receiver*>&, uint8_t payloadType ); //FIXME: send in codec name instead of payload type
    virtual SRef<Encoder_Instance*> create_encoder_instance( uint8_t payloadType );
    void lock_senders() { _senders_lock.lock(); }
    void unlock_senders() { _senders_lock.unlock(); }


    virtual uint32_t get_source_by_name( std :: string name) { return 0; }
    virtual std::list<Receiving_MSS_Reporter *> get_receiving_media_sources() = 0;
    virtual std::list<Sending_MSS_Reporter *> get_sending_media_sources();

protected:
    Realtime_Media( Mini_Sip* _minisip, SRef<Codec_Description *> default_codec );

    Realtime_Media( Mini_Sip* _minisip, const std::list<SRef<Codec_Description *> > & codecList );

    std::list< SRef<Codec_Description *> > _codec_list;
    Mutex _codec_list_lock;

    std::list< SRef<Realtime_Media_Stream_Sender *> > _senders;
    Mutex _senders_lock;
    Mutex _sources_lock;
};

#endif // REALTIME_MEDIA_H
