#include <string.h>

#include "realtime_media.h"
#include "my_time.h"
#include "media_stream.h"

int Receiving_MSS_Reporter::get_received_throughput_kbps()
{
    int result = 0;
    uint64_t now_ms = my_time();
    if(timestampOfLastReceivingThroughtputGet_ms != 0 && now_ms > timestampOfLastReceivingThroughtputGet_ms)
    {
        uint64_t localTotalReceivedPacketSize = totalReceivedPacketSize;
        result = (localTotalReceivedPacketSize - lastReportedTotalReceivedPacketSize) * 8 / (now_ms - timestampOfLastReceivingThroughtputGet_ms);
        lastReportedTotalReceivedPacketSize = localTotalReceivedPacketSize;
    }
    timestampOfLastReceivingThroughtputGet_ms = now_ms;
    return result;
}


Realtime_Media::Realtime_Media( Mini_Sip* _minisip, SRef<Codec_Description *> default_codec )
    : Media(_minisip)
{
    _codec_list.push_back(default_codec);
}

Realtime_Media::Realtime_Media( Mini_Sip* _minisip, const std::list<SRef<Codec_Description *> > & codecList )
    : Media(_minisip)
{
    _codec_list = codecList;
}

Realtime_Media::~Realtime_Media()
{
}

SRef<Codec_Description*> Realtime_Media::get_codec( uint8_t payloadType )
{
    std::list< SRef<Codec_Description *> >::iterator iCodec;

    for( iCodec = _codec_list.begin(); iCodec != _codec_list.end(); iCodec ++ )
    {
        if ( (*iCodec)->get_sdp_media_type() == payloadType )
        {
            return *iCodec;
        }
    }
    return NULL;
}

std::list< SRef<Codec_Description *> >& Realtime_Media::get_available_codecs()
{
    return _codec_list;
}

void Realtime_Media::register_realtime_media_sender( SRef<Realtime_Media_Stream_Sender *> sender )
{
    _senders_lock.lock();
    _senders.push_back( sender );
    _senders_lock.unlock();
}

void Realtime_Media::unregister_realtime_media_sender( SRef<Realtime_Media_Stream_Sender *> sender )
{
    _senders_lock.lock();
    _senders.remove( sender );
    _senders_lock.unlock();
}


void Realtime_Media::send_data_to_streams(const SRef<Processing_Data*>& data)
{
    std::list< SRef<Realtime_Media_Stream_Sender *> >::iterator i;
    _senders_lock.lock();
    for( i = _senders.begin(); i != _senders.end(); i++ )
    {
        //only send if active sender, or if muted only if keep-alive
         if ( (*i)->is_running() )
         {
            if( (*i)->sendonly )
            {// sendonly checks if it is on hold or not it is set with the media attributes sendonly / recvonly / sendrecv
                (*i)->send( data );
            }
        }
    }
    _senders_lock.unlock();
}


void Realtime_Media::send_encoded_data( byte_t * data, uint32_t length, int samplerate, const int64_t &pts_us, bool marker )
{
    std::list< SRef<Realtime_Media_Stream_Sender *> >::iterator i;
    uint32_t ts = (pts_us * samplerate) / 1000000;
    _senders_lock.lock();
    for( i = _senders.begin(); i != _senders.end(); i++ )
    {
        //only send if active sender, or if muted only if keep-alive
         if ( (*i)->is_running() )
         {
            if( (*i)->sendonly )
            {// sendonly checks if it is on hold or not it is set with the media attributes sendonly / recvonly / sendrecv
                if( (*i)->is_muted () )
                {
                    if( (*i)->mute_keep_alive( 50 ) )
                    { //send one packet of every 50
                        memset( data, 0, length );
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    (*i)->send( data, length, &ts, marker );
                }
            }
        }
    }
    _senders_lock.unlock();
}


SRef<Decoder_Instance*> Realtime_Media::create_decoder_instance( const SRef<Realtime_Media_Stream_Receiver*>& rtmsr, uint8_t payloadType )
{
    std::list< SRef<Codec_Description *> >::iterator iC;

    for( iC = _codec_list.begin(); iC != _codec_list.end(); iC ++ )
    {
        if( (*iC)->get_sdp_media_type() == payloadType )
        {
            return (*iC)->new_decoder_instance(rtmsr);
        }
    }
    return NULL;
}


SRef<Encoder_Instance*> Realtime_Media::create_encoder_instance( uint8_t payloadType )
{
    std::list< SRef<Codec_Description *> >::iterator iC;

    for( iC = _codec_list.begin(); iC != _codec_list.end(); iC ++ )
    {
        if( (*iC)->get_sdp_media_type() == payloadType )
        {
            return (*iC)->new_encoder_instance();
        }
    }
    return NULL;
}


std::list<Sending_MSS_Reporter *> Realtime_Media::get_sending_media_sources()
{
    std::list<Sending_MSS_Reporter *> mediaSources;
    for(std::list< SRef<Realtime_Media_Stream_Sender *> >::const_iterator it = _senders.begin(), end = _senders.end(); it != end; ++it)
        mediaSources.push_back(**it);
    return mediaSources;
}
