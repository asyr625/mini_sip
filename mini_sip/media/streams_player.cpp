#include <iostream>
using namespace std;

#include "dbg.h"
#include "my_types.h"
#include "streams_player.h"


Streams_Player::Streams_Player(const unsigned short int &_maxDelay_ms,
                               const unsigned int &bufferOverflowUnderflowModifier_us, const unsigned int &synchronizationToleration_us)
    : _max_delay_ms(_maxDelay_ms),
      _buffer_overflow_underflow_modifier_us(bufferOverflowUnderflowModifier_us * 1000),
      _synchronization_toleration_us(synchronizationToleration_us * 1000),
      _audio_stream_player_factory(NULL), _video_stream_player_factory(NULL)
{

}

Stream_Player* Streams_Player::add_audio_stream_player(Audio_Media_Source *_sink, const int &_rtpTimestampSamplingRate,
                                                       const std::string &callID, const unsigned int &ssrc)
{
    if(_audio_stream_player_factory)
    {
        //std::cout << "Streams_Player::add_audio_stream_player() ok" << std::endl;
        return *(*_stream_players.insert(
            std::make_pair(
                StreamsMapKey(ssrc, callID),
                _audio_stream_player_factory->create_audio_stream_player(this, _sink, _rtpTimestampSamplingRate,
                                                                         _buffer_overflow_underflow_modifier_us,
                                                                         _synchronization_toleration_us, AUDIO_STREAM_PLAYER_INTRA_WAKE_PERIOD_ms)
            )
        ).first).second;
    }
    my_err << "Streams_Player::add_audio_stream_player() error, _audio_stream_player_factory was NULL" << std::endl;
    return NULL;
}

Stream_Player* Streams_Player::add_video_stream_player(Image_Handler *_sink, const int &_rtpTimestampSamplingRate,
                                                       const std::string &callID, const unsigned int &ssrc)
{
    if(_video_stream_player_factory)
    {
        //std::cout << "Streams_Player::add_video_stream_player() ok" << std::endl;
        return *(*_stream_players.insert(
            std::make_pair(
                StreamsMapKey(ssrc, callID),
                _video_stream_player_factory->create_video_stream_player(this, _sink, _rtpTimestampSamplingRate,
                                                                         _buffer_overflow_underflow_modifier_us,
                                                                         _synchronization_toleration_us)
            )
        ).first).second;
    }
    my_err << "Streams_Player::add_video_stream_player() error, _video_stream_player_factory was NULL" << std::endl;
    return NULL;
}

void Streams_Player::remove(Stream_Player* _stream_player)
{
    _stream_player->stop();
    for(Stream_Players::iterator it = _stream_players.begin(), end = _stream_players.end(); it!=end; ++it)
    {
        if(*it->second == _stream_player)
        {
            _stream_players.erase(it);
            //std::cout << "One StreamPlayer erased, there are still " << streamPlayers.size() << " left" << std::endl;
            return;
        }
    }
    my_err << "Streams_Player::remove() could not locate a stream player to remove" << std::endl;
}

void Streams_Player::add_rtcp_sender_report_timestamps(const std::string &callID, const unsigned int &ssrc,
                                                       const int64_t &ntp_usecsSinceJan1st1970,
                                                       const unsigned int &rtp_timestamp)
{
    StreamsMapKey streamsMapKey(ssrc, callID);
    Stream_Players::iterator it = _stream_players.find(streamsMapKey);
    if(it != _stream_players.end())
    {
        int64_t monotonicRtpTimestamp_us = it->second->monotonic_rtp_timestamp_us(rtp_timestamp);
        int64_t *&grabbedTimestampToRtc_us = it->second->_grabbed_timestamp_to_rtc_us;

        if(grabbedTimestampToRtc_us == NULL)
            grabbedTimestampToRtc_us = new int64_t;
        *grabbedTimestampToRtc_us = ntp_usecsSinceJan1st1970 - monotonicRtpTimestamp_us;
        my_dbg << "Rtcp Sender Report (ssrc = " << ssrc << ", callID = " << callID << ") says that " << (uint64_t)(monotonicRtpTimestamp_us / 1000) << "ms was taken at " << (uint64_t)(ntp_usecsSinceJan1st1970 / 1000) << "ms" << std::endl;
        it->second->measure_on_next_arrival();
    }
    else
        my_err << "Streams_Player::add_rtcp_sender_report_timestamps() could not find proper stream player" << std::endl;
}

unsigned short Streams_Player::get_max_delay_ms()
{
    return _max_delay_ms;
}

void Streams_Player::set_max_delay_ms(const unsigned short int &_maxDelay_ms)
{
    _max_delay_ms = _maxDelay_ms;
}

void Streams_Player::synchronize_streams()
{
    _stream_synchronization_mutex.lock();
    int delaySum_us=0, count=0;
    for(Stream_Players::iterator it = _stream_players.begin(), end = _stream_players.end(); it!=end; ++it)
    {
        if(it->second->_grabbed_timestamp_to_rtc_us && it->second->_enqueued_timestamp_to_rtc_us)
        {
            delaySum_us += *it->second->_enqueued_timestamp_to_rtc_us - *it->second->_grabbed_timestamp_to_rtc_us;
            ++count;
        }
    }

    if(count > 1)
    {
        int meanDelay_us = delaySum_us / count;
        for(Stream_Players::iterator it = _stream_players.begin(), end = _stream_players.end(); it!=end; ++it)
            if(it->second->_grabbed_timestamp_to_rtc_us && it->second->_enqueued_timestamp_to_rtc_us)
                it->second->order_delay(_max_delay_ms * 1000 / 2 + *it->second->_enqueued_timestamp_to_rtc_us - *it->second->_grabbed_timestamp_to_rtc_us - meanDelay_us);
    }
    _stream_synchronization_mutex.unlock();
}

void Streams_Player::set_audio_stream_player_factory(Audio_Stream_Player_Factory *factory)
{
    _audio_stream_player_factory = factory;
}

void Streams_Player::set_video_stream_player_factory(Video_Stream_Player_Factory *factory)
{
    _video_stream_player_factory = factory;
}
