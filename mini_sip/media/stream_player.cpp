#include "stream_player.h"

Stream_Player::Stream_Player(IStream_To_Streams_Player *owner, const std::string &_streamType, const int &_rtpTimestampSamplingRate, const unsigned int &bufferOverflowUnderflowModifier_us, const unsigned int &synchronizationToleration_us)
    : Runnable(),
      _owner(owner), type(_streamType), _thread(NULL), _quit(false),
      _rtp_timestamp_sampling_rate(_rtpTimestampSamplingRate), _ordered_delay_us(-1),
      _measure_on_arrival(false), _grabbed_timestamp_to_rtc_us(NULL), _enqueued_timestamp_to_rtc_us(NULL),
      BUFFER_OVERFLOW_UNDERFLOW_MODIFIER_us(bufferOverflowUnderflowModifier_us),
      SYNCHRONIZATION_TOLERATION_us(synchronizationToleration_us)
{
    my_assert( _rtpTimestampSamplingRate );
}


Stream_Player::~Stream_Player()
{
    delete _grabbed_timestamp_to_rtc_us;
    delete _enqueued_timestamp_to_rtc_us;
    delete _thread;
}

void Stream_Player::stop(const bool &block_until_finished)
{
    _quit = true;
    if(block_until_finished && _thread)
        _thread->join();
}

void Stream_Player::order_delay(const int &delay_us)
{
    _ordered_delay_us = std::min(std::max(0, delay_us), _owner->get_max_delay_ms() * 1000);
}

void Stream_Player::measure_on_next_arrival()
{
    _measure_on_arrival = true;
}

int64_t Stream_Player::monotonic_rtp_timestamp_us(const uint32_t &rtp_timestamp)
{
    return (_rtp_timestamp_monotonizer.monotonic(rtp_timestamp) * 1000000) / _rtp_timestamp_sampling_rate;
}
