#ifndef AUDIO_STREAM_PLAYER_H
#define AUDIO_STREAM_PLAYER_H
#include <deque>
#include "stream_player.h"

class Audio_Media_Source;

struct Audio_Stream_Player_Queue_Element
{
    int64_t timestamp_us;
    short *buffer;
    int nSamples;
    int seqNo;
    int sfreq;
    bool stereo;

    Audio_Stream_Player_Queue_Element(const int64_t &_timestamp_us=-1, short *_buffer=NULL, const int &_nSamples=0, const int &_seqNo=0, const int &_sfreq=0, const bool &_stereo=false)
        : timestamp_us(_timestamp_us), buffer(_buffer), nSamples(_nSamples), seqNo(_seqNo), sfreq(_sfreq), stereo(_stereo)
    {
    }
};

class Audio_Stream_Player : public virtual Stream_Player
{
public:
    Audio_Stream_Player(IStream_To_Streams_Player *_owner, Audio_Media_Source *sink,
                        const int &_rtpTimestampSamplingRate,
                        const unsigned int &bufferOverflowUnderflowModifier_us,
                        const unsigned int &synchronizationToleration_us,
                        const unsigned int &intraWakePeriod_ms);

    virtual ~Audio_Stream_Player();
    virtual void run();
    void enqueue(const unsigned int &rtpTimestamp, short *samples, const int32_t &nSamples, const int32_t &index, const int &freq, const bool &isStereo, bool rtpMarker);
protected:
    Audio_Media_Source *_sink;
    unsigned int _intra_wake_period_ms;
    int64_t _playback_time_us;
    unsigned int _buffer_underflow_count;
    int64_t _run_wake_up_time_us;
    std::deque<Audio_Stream_Player_Queue_Element> queue;
};

#endif // AUDIO_STREAM_PLAYER_H
