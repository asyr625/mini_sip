#include "video_stream_player.h"

#include "streams_player.h"
#include "video_display.h"
#include "dbg.h"
#include "my_time.h"

Video_Stream_Player::Video_Stream_Player(IStream_To_Streams_Player *_owner, SRef<Image_Handler *> _sink,
                                         const int &_rtpTimestampSamplingRate, const unsigned int &bufferOverflowUnderflowModifier_us,
                                         const unsigned int &synchronizationToleration_us)
    : Stream_Player(_owner, "Video", _rtpTimestampSamplingRate, bufferOverflowUnderflowModifier_us, synchronizationToleration_us),
      sink(_sink), timestampOfLastQueuedFrame_us(0),
      fromTimestampToPlaybackRTC_us(0)
{
}

Video_Stream_Player::~Video_Stream_Player()
{
    _queue_mutex.lock();
    while(!queue.empty())
    {
        std::deque<VideoStreamPlayerQueueElement>::iterator it=queue.begin();
        it->image = NULL;
        queue.erase(it);
    }
    _queue_mutex.unlock();
}

void Video_Stream_Player::handle(const SRef<SImage*>& image)
{
    SRef<Processing_Data_Video*> vdata = new Processing_Data_Video;
    vdata->image = image;

    handle_data(*vdata);
}

void Video_Stream_Player::handle_data(const SRef<Processing_Data*>& data)
{
    SRef<Processing_Data_Video*> vdata = (Processing_Data_Video*)*data;
    SRef<SImage*> image = vdata->image;

    _time_of_last_enqueue_us = utime();
    image->uTime = monotonic_rtp_timestamp_us((unsigned int)image->uTime);
    timestampOfLastQueuedFrame_us = image->uTime;
    VideoStreamPlayerQueueElement queueElement(image);

    int64_t rtc_us = 0;
    if(_measure_on_arrival) // multiple simultaneous executions appear impossible
    {
        rtc_us = utime(true);
        my_dbg << "It's " << (uint64_t)(rtc_us / 1000) << "ms now" << std::endl;
    }

    _queue_mutex.lock();
    drop_oldest_non_resizing_queued_frame_if_older_than(2 * 1000 * _owner->get_max_delay_ms());
    queue.push_back(queueElement);
    _queue_mutex.unlock();

    if(_thread == NULL)
    {
        fromTimestampToPlaybackRTC_us = _time_of_last_enqueue_us - timestampOfLastQueuedFrame_us + _owner->get_max_delay_ms() * 1000 / 2; // fill the buffer to around the half of its length
        _thread = new Thread(this, Thread::Normal_Priority);
    }

    my_dbg << "Video_Stream_Player is queued frame " << (uint64_t)(timestampOfLastQueuedFrame_us / 1000) << "ms" << std::endl;

    if(_measure_on_arrival && rtc_us != 0) // multiple simultaneous executions appear impossible
    {
        _measure_on_arrival = false;
        if(_enqueued_timestamp_to_rtc_us == NULL)
            _enqueued_timestamp_to_rtc_us = new int64_t;
        *_enqueued_timestamp_to_rtc_us = rtc_us - timestampOfLastQueuedFrame_us;
        _owner->synchronize_streams();
    }
}


void Video_Stream_Player::run()
{
#ifdef DEBUG_OUTPUT
    set_thread_name("VideoStreamPlayer::run");
#endif
    while(!_quit)
    {
        //my_dbg << "Video_Stream_Player::run() started at " << mtime() << "ms" << std::endl;
        bool wasEmpty = false;
        while(queue.empty())
        {
            if(_quit)
                return;
            my_sleep(BUFFER_OVERFLOW_UNDERFLOW_MODIFIER_us / 1000);
            wasEmpty = true;
        }
        _queue_mutex.lock();
        my_assert(!queue.empty());
        VideoStreamPlayerQueueElement queueElement = queue.front();
        queue.pop_front();
        _queue_mutex.unlock();

        my_dbg << "Video_Stream_Player popped frame " << (uint64_t)(queueElement.image->uTime / 1000) << "ms" << std::endl;
        int64_t sleepTime_us = queueElement.image->uTime + fromTimestampToPlaybackRTC_us - utime();
        if(sleepTime_us >= 1000)
        {
            //my_dbg << "Video_Stream_Player sleeping " << sleepTime_us / 1000 << "ms" << std::endl;
            my_sleep(sleepTime_us / 1000);
        }
        else
        {
            if(wasEmpty && sleepTime_us < 0)
            {
                fromTimestampToPlaybackRTC_us -= sleepTime_us;
                my_err << "VideoStreamPlayer has " << (int)(-sleepTime_us / 1000) << "ms buffer underrun, modyfying timestamp-to-playback-RTC accordingly" << std::endl;
            }
        }

        int delay_us = timestampOfLastQueuedFrame_us - queueElement.image->uTime + utime() - _time_of_last_enqueue_us;

        bool bufferOverflow = (delay_us >= _owner->get_max_delay_ms() * 1000);
        if(bufferOverflow)
        {
            fromTimestampToPlaybackRTC_us -= BUFFER_OVERFLOW_UNDERFLOW_MODIFIER_us;
            my_dbg << "VideoStreamPlayer buffer overflow, modyfying timestamp-to-playback-RTC by " << BUFFER_OVERFLOW_UNDERFLOW_MODIFIER_us / 1000 << "ms" << std::endl;
        }
        if(_ordered_delay_us >= 0)
        {
            int delayDiff_us = _ordered_delay_us - delay_us;
            if(abs(delayDiff_us) > SYNCHRONIZATION_TOLERATION_us)
            {
                fromTimestampToPlaybackRTC_us += delayDiff_us;
                my_dbg << "VideoStreamPlayer applying ordered delay, modifying timestamp-to-playback-RTC by " << delayDiff_us / 1000 << "ms" << std::endl;
            }
            _ordered_delay_us = -1;
        }
        uint64_t imageTime_ms = queueElement.image->uTime / 1000;

        sink->handle(queueElement.image);

        my_dbg << "VideoStreamPlayer shown frame " << imageTime_ms << "ms" << std::endl; // at " << mtime() << "ms" << std::endl;
    }
}

void Video_Stream_Player::drop_oldest_non_resizing_queued_frame_if_older_than(const unsigned int &olderThan_us)
{
    std::deque<VideoStreamPlayerQueueElement>::iterator it = queue.begin();
    if(it != queue.end() && timestampOfLastQueuedFrame_us - it->image->uTime > olderThan_us)
    {
        if(it != queue.end())
        {
            std::deque<VideoStreamPlayerQueueElement>::iterator next = it;
            ++next;
            if(next != queue.end())
                fromTimestampToPlaybackRTC_us -= next->image->uTime - it->image->uTime;
            my_err << "Video_Stream_Player critical buffer overflow, dropping frame " << (uint64_t)(it->image->uTime / 1000) << "ms" << std::endl;
            //SImage::destroy(it->image);
            it->image = NULL;
            queue.erase(it);
        }
    }
}
