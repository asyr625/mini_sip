#ifndef STREAM_PLAYER_H
#define STREAM_PLAYER_H

#include "mutex.h"
#include "thread.h"
#include "my_types.h"
#include "rollover_prone_to_monotonic.h"

class Streams_Player;

struct IStream_To_Streams_Player
{
    virtual unsigned short int get_max_delay_ms() = 0;
    virtual void synchronize_streams() = 0;
};

class Stream_Player : public virtual Runnable
{
public:
    friend class Streams_Player;

    Stream_Player(IStream_To_Streams_Player *_owner, const std::string &_streamType,
                  const int &_rtpTimestampSamplingRate,
                  const unsigned int &bufferOverflowUnderflowModifier_us,
                  const unsigned int &synchronizationToleration_us);

    virtual ~Stream_Player();
    virtual void stop(const bool &block_until_finished=true);
    virtual void order_delay(const int &delay_us);
    virtual void measure_on_next_arrival();
protected:
    virtual int64_t monotonic_rtp_timestamp_us(const uint32_t &rtp_timestamp);

    IStream_To_Streams_Player *_owner;
    Mutex _queue_mutex;
    Thread *_thread;
    bool _quit;
    int _rtp_timestamp_sampling_rate;
    int _ordered_delay_us;
    Rollover_Prone_To_Monotonic<uint32_t, int64_t> _rtp_timestamp_monotonizer;
    bool _measure_on_arrival;
    int64_t *_grabbed_timestamp_to_rtc_us;
    int64_t *_enqueued_timestamp_to_rtc_us;
    int64_t _time_of_last_enqueue_us;

    const unsigned int BUFFER_OVERFLOW_UNDERFLOW_MODIFIER_us;
    const unsigned int SYNCHRONIZATION_TOLERATION_us;
private:
    std::string type;
};

#endif // STREAM_PLAYER_H
