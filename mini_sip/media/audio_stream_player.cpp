
#include <string.h>

#include "audio_stream_player.h"
#include "my_time.h"
#include "audio_media.h"

#include "dbg.h"
#include "my_types.h"

Audio_Stream_Player::Audio_Stream_Player(IStream_To_Streams_Player *_owner, Audio_Media_Source *sink,
                                         const int &_rtpTimestampSamplingRate,
                                         const unsigned int &bufferOverflowUnderflowModifier_us,
                                         const unsigned int &synchronizationToleration_us,
                                         const unsigned int &intraWakePeriod_ms)
    : Stream_Player(_owner, "Audio", _rtpTimestampSamplingRate, bufferOverflowUnderflowModifier_us, synchronizationToleration_us),
      _sink(sink), _intra_wake_period_ms(std::min(intraWakePeriod_ms, uint32_t(_owner->get_max_delay_ms() / 2))),
      _playback_time_us(0), _buffer_underflow_count(0),
      _run_wake_up_time_us(0)
{
}

Audio_Stream_Player::~Audio_Stream_Player()
{
    _queue_mutex.lock();
    while(!queue.empty())
    {
        std::deque<Audio_Stream_Player_Queue_Element>::iterator it=queue.begin();
        delete[] it->buffer;
        queue.erase(it);
    }
    _queue_mutex.unlock();
}

void Audio_Stream_Player::enqueue(const unsigned int &rtpTimestamp, short *samples,
                                  const int32_t &nSamples, const int32_t &index,
                                  const int &freq, const bool &isStereo, bool rtpMarker)
{
    _time_of_last_enqueue_us = utime();
    _rtp_timestamp_sampling_rate = freq;
    int64_t timestamp_us = monotonic_rtp_timestamp_us(rtpTimestamp);
    short *localSamples = new short[(nSamples << isStereo) * sizeof(short)];
    memcpy(localSamples, samples, (nSamples << isStereo) * sizeof(short));
    //	my_out << "Audio_Stream_Player enqueue() locking the mutex" << std::endl;
    int64_t rtc_us = 0;
    if(_measure_on_arrival) // multiple simultaneous executions appear impossible
    {
        rtc_us = utime(true);
        my_dbg << "It's " << (uint64_t)(rtc_us / 1000) << "ms now" << std::endl;
    }
    _queue_mutex.lock();

    if (rtpMarker && _thread)   // first packet after a silence period
    {
        _playback_time_us = timestamp_us + (_owner->get_max_delay_ms() / 2 - _intra_wake_period_ms) * int64_t(1000);
    }

    queue.push_back(Audio_Stream_Player_Queue_Element(timestamp_us, localSamples, nSamples, index, freq, isStereo));
    if(_thread == NULL)
    {
        _playback_time_us = timestamp_us + (_owner->get_max_delay_ms() / 2 - _intra_wake_period_ms) * int64_t(1000);
        _thread = new Thread(this, Thread::Above_Normal_Priority);
    }

    //my_out << "Audio_Stream_Player enqueue() unlocking the mutex" << std::endl;
    _queue_mutex.unlock();
    my_dbg << "Audio_Stream_Player queued frame " << (uint64_t)(timestamp_us / 1000) << "ms" << std::endl;

    if(_measure_on_arrival && rtc_us != 0) // multiple simultaneous executions appear impossible
    {
        _measure_on_arrival = false;
        if(_enqueued_timestamp_to_rtc_us == NULL)
            _enqueued_timestamp_to_rtc_us = new int64_t;
        *_enqueued_timestamp_to_rtc_us = rtc_us - timestamp_us;
        _owner->synchronize_streams();
    }
}

void Audio_Stream_Player::run()
{
#ifdef DEBUG_OUTPUT
    set_thread_name("Audio_Stream_Player::run");
#endif
    //	my_out << "Audio_Stream_Player run() started, sleeping " << (owner->getMaxDelay_ms() + intraWakePeriod_ms) / 2 << "ms" << std::endl;
    my_sleep(_owner->get_max_delay_ms() / 2);
    _run_wake_up_time_us = utime();
    while(!_quit)
    {
        int delay_us = utime() - _time_of_last_enqueue_us;
        _queue_mutex.lock();
        if(!queue.empty())
            delay_us += queue.back().timestamp_us - queue.front().timestamp_us;
        //my_out << "Audio_Stream_Player run() locking the mutex" << std::endl;
        while(1)
        {
            bool queueEmpty = queue.empty();
            if(queueEmpty)
            {
                //my_out << "Audio_Stream_Player run() unlocking the mutex due to empty queue" << std::endl;
                _queue_mutex.unlock();
            }
            while(queue.empty())
            {
                if(_quit)
                    return;
                if(_buffer_underflow_count++ % (1000000 / BUFFER_OVERFLOW_UNDERFLOW_MODIFIER_us) == 0)
                    my_dbg << "Audio_Stream_Player buffer underflow, moving the wake timer forward" << std::endl;

                _run_wake_up_time_us += BUFFER_OVERFLOW_UNDERFLOW_MODIFIER_us;
                my_sleep(BUFFER_OVERFLOW_UNDERFLOW_MODIFIER_us / 1000);
            }

            _buffer_underflow_count = 0;
            if(queueEmpty)
            {
                //my_out << "Audio_Stream_Player run() relocking the mutex due to empty queue" << std::endl;
                _queue_mutex.lock();
            }
            Audio_Stream_Player_Queue_Element &first = queue.front();
            if(first.timestamp_us > _playback_time_us)
            {
                //my_out << "Audio_Stream_Player run() breaking" << std::endl;
                break;
            }
            my_dbg << "Audio_Stream_Player popped frame " << (uint64_t)(first.timestamp_us / 1000) << "ms" << std::endl;
            _sink->push_sound(first.buffer, first.nSamples, first.seqNo, first.sfreq, first.stereo);
            delete[] first.buffer;
            queue.pop_front();
        }
        //my_out << "Audio_Stream_Player run() unlocking the mutex" << std::endl;
        _queue_mutex.unlock();
        _run_wake_up_time_us += _intra_wake_period_ms * 1000;
        bool bufferOverflow = (delay_us >= 1000 * _owner->get_max_delay_ms());
        if(bufferOverflow)
        {
            _playback_time_us += BUFFER_OVERFLOW_UNDERFLOW_MODIFIER_us;
            //merr << "Audio_Stream_Player buffer overflow, moving the playback timer forward by " << BUFFER_OVERFLOW_UNDERFLOW_MODIFIER_us / 1000 << "ms" << std::endl;
        }
        if(_ordered_delay_us >= 0)
        {
            int delayDiff_us = _ordered_delay_us - delay_us;
            if(abs(delayDiff_us) > SYNCHRONIZATION_TOLERATION_us)
            {
                _playback_time_us -= delayDiff_us;
                my_dbg << "Audio_Stream_Player applying ordered delay, modifying the playback timer by " << -delayDiff_us / 1000 << "ms" << std::endl;
            }
            _ordered_delay_us = -1;
        }
        _playback_time_us += _intra_wake_period_ms * 1000;
        int sleepTime_ms = (_run_wake_up_time_us - int64_t(utime())) / 1000;
        //my_dbg << "Audio_Stream_Player run() sleeping " << _intra_wake_period_ms << "ms" << std::endl;
        if(sleepTime_ms > 0)
            my_sleep(sleepTime_ms);
    }
}
