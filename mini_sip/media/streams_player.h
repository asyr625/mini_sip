#ifndef STREAMS_PLAYER_H
#define STREAMS_PLAYER_H
#include <map>
#include "stream_player.h"

class Audio_Stream_Player;
class Video_Stream_Player;
class Audio_Media_Source;
class Image_Handler;

struct Audio_Stream_Player_Factory
{
    virtual Stream_Player* create_audio_stream_player(IStream_To_Streams_Player *_owner,
                                                      Audio_Media_Source *_sink,
                                                      const int &_rtpTimestampSamplingRate,
                                                      const unsigned int &bufferOverflowUnderflowModifier_us,
                                                      const unsigned int &synchronizationToleration_us,
                                                      const unsigned int &_intraWakePeriod_ms) = 0;
};

struct Video_Stream_Player_Factory
{
    virtual Stream_Player* create_video_stream_player(IStream_To_Streams_Player *_owner,
                                                      Image_Handler *_sink,
                                                      const int &_rtpTimestampSamplingRate,
                                                      const unsigned int &bufferOverflowUnderflowModifier_us,
                                                      const unsigned int &synchronizationToleration_us) = 0;
};

struct IStreams_Player_Report_Timestamps
{
    virtual void add_rtcp_sender_report_timestamps(const std::string &callID,
                                                   const unsigned int &ssrc,
                                                   const int64_t &ntp_usecsSinceJan1st1970,
                                                   const unsigned int &rtp_timestamp) = 0;
};

#define AUDIO_STREAM_PLAYER_INTRA_WAKE_PERIOD_ms 10


class Streams_Player : public IStream_To_Streams_Player, public IStreams_Player_Report_Timestamps, public SObject
{
public:
    Streams_Player(const unsigned short int &_maxDelay_ms,
                   const unsigned int &bufferOverflowUnderflowModifier_us, const unsigned int &synchronizationToleration_us);

    Stream_Player* add_audio_stream_player(Audio_Media_Source *_sink,
                                           const int &_rtpTimestampSamplingRate,
                                           const std::string &callID,
                                           const unsigned int &ssrc);

    Stream_Player* add_video_stream_player(Image_Handler *_sink,
                                           const int &_rtpTimestampSamplingRate,
                                           const std::string &callID,
                                           const unsigned int &ssrc);

    void remove(Stream_Player* _stream_player);

    virtual void add_rtcp_sender_report_timestamps(const std::string &callID,
                                                   const unsigned int &ssrc,
                                                   const int64_t &ntp_usecsSinceJan1st1970,
                                                   const unsigned int &rtp_timestamp);

    virtual unsigned short int get_max_delay_ms();
    void set_max_delay_ms(const unsigned short int &_maxDelay_ms);
    virtual void synchronize_streams();
    void set_audio_stream_player_factory(Audio_Stream_Player_Factory *factory);
    void set_video_stream_player_factory(Video_Stream_Player_Factory *factory);

protected:
    typedef std::pair<unsigned int, std::string> StreamsMapKey;
    typedef std::map<StreamsMapKey, SRef<Stream_Player *> > Stream_Players;
    Stream_Players _stream_players;
    unsigned short int _max_delay_ms;
    unsigned int _buffer_overflow_underflow_modifier_us;
    unsigned int _synchronization_toleration_us;
    Mutex _stream_synchronization_mutex;
    Audio_Stream_Player_Factory *_audio_stream_player_factory;
    Video_Stream_Player_Factory *_video_stream_player_factory;
};

#endif // STREAMS_PLAYER_H
