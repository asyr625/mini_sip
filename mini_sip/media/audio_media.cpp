#include "audio_media.h"

#include "rtp_header.h"
#include "media_stream.h"
#include "file_sound_source.h"
#include "resampler.h"
#include "sound_source.h"
#include "audio_stream_player.h"
#include "streams_player.h"
#include "rtp_packet.h"

#include "my_time.h"

#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<iostream>
#include<stdio.h>

#ifdef _MSC_VER

#else
#include<sys/time.h>
#include<unistd.h>
#endif

#include<string.h> //for memset

#ifdef DEBUG_OUTPUT
#include "string_utils.h"
#endif

class G711_Codec;

#ifdef AEC_SUPPORT
AEC AudioMedia::aec;		//hanning
#endif

#ifdef _WIN32_WCE
#	include "minisip_wce_extra_includes.h"
#endif

using namespace std;

#define RINGTONE_SOURCE_ID 0x42124212

#define SOUND_CARD_FREQ 16000

Audio_Media::Audio_Media(Mini_Sip*_minisip, SRef<Sound_IO *> soundIo, const std::list<SRef<Codec_Description *> > & codecList, Streams_Player *_streamsPlayer )
    : Realtime_Media(_minisip, codecList), _streams_player(_streamsPlayer)
{
    _sound_io = soundIo;

    receive = true;
    send = true;

    _sound_io->register_recorder_receiver( this, SOUND_CARD_FREQ * 20 / 1000, false );

    _seq_no = 0;

    _resampler = Resampler_Registry::get_instance()->create( SOUND_CARD_FREQ, 8000, 20, 1 /*Nb channels */);
    _streams_player->set_audio_stream_player_factory(this);

    _processing_audio_device_sender = new Realtime_Media_Pipeline( _mini_sip, NULL,  NULL , this, "audio", PROCESSOR_DIRECTION_DEV2NET, PROCESSOR_INSERT_POINT_DEVICE );
    Processor_Registry::get_instance()->add_plugins( _processing_audio_device_sender, NULL);
}


std::string Audio_Media::get_sdp_media_type()
{
    return "audio";
}

void Audio_Media::register_realtime_media_sender( SRef<Realtime_Media_Stream_Sender *> sender )
{
    _senders_lock.lock();
    if( _senders.empty() )
    {
        _senders_lock.unlock();
        _sound_io->start_record();
        _senders_lock.lock();
    }

    bool found = false;
    std::list<SRef<Realtime_Media_Stream_Sender *> >::iterator i;
    for (i=_senders.begin(); i != _senders.end(); i++)
    {
        if ( *i == sender)
            found = true;
    }
    if (!found)
        _senders.push_back( sender );
    _senders_lock.unlock();
}

void Audio_Media::unregister_realtime_media_sender( SRef<Realtime_Media_Stream_Sender *> sender )
{
    bool emptyList;
    _senders_lock.lock();
    _senders.remove( sender );
    emptyList = _senders.empty();
    _senders_lock.unlock();

    if( emptyList )
    {
        _sound_io->stop_record();
    }
}

void Audio_Media::register_media_source( const SRef<Session*>& session, uint32_t ssrc,
                                         std::string callId, SRef<Realtime_Media_Stream_Receiver*> rtMSR )
{
    SRef<Audio_Media_Source *> source;
    source = new Audio_Media_Source(session, ssrc, callId, this, _streams_player, rtMSR );
    _sound_io->register_source( *source );
    _sources.push_back( source );
}

void Audio_Media::unregister_media_source( uint32_t ssrc )
{
    std::list< SRef<Audio_Media_Source *> >::iterator iSource;

    _sound_io->unregister_source( ssrc );

    for( iSource = _sources.begin(); iSource != _sources.end(); iSource ++ )
    {
        if( (*iSource)->get_ssrc() == ssrc )
        {
            _sources.erase( iSource );
            return;
        }
    }
}

void Audio_Media::play_data( const SRef<Rtp_Packet *> & packet )
{
    SRef<Audio_Media_Source *> source = get_source( packet->get_header().ssrc );

    if( source )
    {
        source->play_data( packet );
    }
}

void Audio_Media::srcb_handle_sound( void *data, int nsamples, int samplerate )
{
    SRef<Processing_Data_Audio*> audio = new Processing_Data_Audio(false);
    audio->samples = (short*)data;
    audio->nsamples = nsamples;
    audio->sfreq = samplerate;
    audio->nchannels = 1;
    audio->rtp_seq_no = _seq_no;
    audio->_rtp_timestamp = utime();
    audio->rtp_marker = false;
    audio->ssrc=0;
    ++_seq_no;

    _processing_audio_device_sender->process(*audio);
}

#ifdef AEC_SUPPORT
void Audio_Media::srcb_handle_sound( void *data, int length, void *dataR)	//hanning
{
    _resampler->resample( (short *)data, resampledData );
    _resampler->resample( (short *)dataR, resampledDataR );

    for(int j=0; j<160; j++){
        _resampled_data[j] = (short)aec.do_aec((int)_resampled_data[j], (int)resampled_dataR[j]);
    }
    send_data( (byte_t*) &_resampled_data, 160*sizeof(short), 0, false );
    _seq_no ++;
}
#endif

void Audio_Media::handle_data(const SRef<Processing_Data*>& a)
{
    Processing_Data_Audio* audio = (Processing_Data_Audio*)*a;
    const int64_t pts_us = audio->_rtp_timestamp;
    uint32_t nsamples= audio->nsamples;
    byte_t*data = (byte_t*)audio->samples;
    int samplerate = audio->sfreq;
    bool marker = audio->rtp_marker;

    send_data_to_streams(a);
}

void Audio_Media::start_ringing( std::string ringtoneFile )
{
    _sound_io->register_source( new File_Sound_Source( "", ringtoneFile, RINGTONE_SOURCE_ID, 44100, 2, SOUND_CARD_FREQ, 20, 2, true ) );
}

void Audio_Media::stop_ringing()
{
    _sound_io->unregister_source( RINGTONE_SOURCE_ID );
}

std::string Audio_Media::get_debug_string()
{
    std::string ret;
#ifdef DEBUG_OUTPUT
    ret = get_mem_object_type() + ": this=" + itoa(reinterpret_cast<int64_t>(this));
    for( std::list< SRef<Realtime_Media_Stream_Sender *> >::iterator it = _senders.begin();
         it != _senders.end(); it++ ) {
        ret += (*it)->get_debug_string() + ";";
    }
#endif
    return ret;
}

SRef<Audio_Media_Source *> Audio_Media::get_source( uint32_t ssrc )
{
    std::list<SRef<Audio_Media_Source *> >::iterator i;

    for( i = _sources.begin(); i != _sources.end(); i++ )
    {
        if( (*i)->get_ssrc() == ssrc )
            return (*i);
    }
    return NULL;
}


std::list<Receiving_MSS_Reporter *> Audio_Media::get_receiving_media_sources()
{
    std::list<Receiving_MSS_Reporter *> mediaSources;
    for(std::list< SRef<Audio_Media_Source *> >::const_iterator it = _sources.begin(), end = _sources.end(); it!=end; ++it)
        mediaSources.push_back(**it);
    return mediaSources;
}

Stream_Player* Audio_Media::create_audio_stream_player(IStream_To_Streams_Player *_owner, Audio_Media_Source *_sink,
                                                       const int &_rtpTimestampSamplingRate,
                                                       const unsigned int &bufferOverflowUnderflowModifier_us,
                                                       const unsigned int &synchronizationToleration_us,
                                                       const unsigned int &_intraWakePeriod_ms)
{
    return new Audio_Stream_Player(_owner, _sink, _rtpTimestampSamplingRate, bufferOverflowUnderflowModifier_us,
                                   synchronizationToleration_us, _intraWakePeriod_ms);
}



Audio_Media_Source::Audio_Media_Source( const SRef<Session*>& session, uint32_t ssrc, std::string callId, SRef<Media *> media,
                                        Streams_Player *_streamsPlayer, SRef<Realtime_Media_Stream_Receiver*> rtMSR )
    :Basic_Sound_Source( ssrc, callId, NULL, //plc
                         0/*position*/, SOUND_CARD_FREQ,
                         20, //duration in ms
                         2 //number of channels (numChannels)
                         //buffer size defaults to 16000 * numChannels
                         ),
      _media(media),
      _ssrc(ssrc),
      _streams_player(_streamsPlayer),
      _audio_stream_player(dynamic_cast<Audio_Stream_Player *>(_streamsPlayer->add_audio_stream_player(this, 16000, callId, ssrc))),
      _last_packet_seq_no( 0 ),
      _realtime_stream(rtMSR)
{
    _processing_audio = new Realtime_Media_Pipeline( session->get_mini_sip(), session, *rtMSR , this, "audio", PROCESSOR_DIRECTION_NET2DEV, PROCESSOR_INSERT_POINT_STREAM );
    Processor_Registry::get_instance()->add_plugins( _processing_audio, *rtMSR );
}

Audio_Media_Source::~Audio_Media_Source()
{
    _streams_player->remove(_audio_stream_player);
}

void Audio_Media_Source::play_data( const SRef<Rtp_Packet *> & rtpPacket )
{
    ++receivedPacketCount;
    Rtp_Header &hdr = rtpPacket->get_header();
    totalReceivedPacketSize += hdr.size() + rtpPacket->get_content_length();
    uint16_t packetSeqNoDiff = hdr.get_seq_no() - _last_packet_seq_no;

    if(receivedPacketCount > 1 && packetSeqNoDiff > 1 && packetSeqNoDiff < uint16_t(0x1 << 14))
    {
        lostPacketCount += packetSeqNoDiff - 1;
    }
    _last_packet_seq_no = hdr.get_seq_no();

    SRef<Decoder_Instance*> codec = find_decoder( hdr.get_payload_type() );
    if( codec )
    {
        SRef<Processing_Data*> data =codec->decode(rtpPacket);
        _processing_audio->process(*data);
    }
}

void Audio_Media_Source::handle_data(const SRef<Processing_Data*>& data )
{
    SRef<Processing_Data_Audio*> adata = (Processing_Data_Audio*)*data;

    //FIXME: rtpSeqNo is not used - remove from parameters
    push_sound(adata->samples, adata->nsamples, adata->rtp_seq_no, adata->sfreq, false);
}

uint32_t Audio_Media_Source::get_ssrc()
{
    return _ssrc;
}


SRef<Decoder_Instance*> Audio_Media_Source::find_decoder( uint8_t payloadType )
{
    std::list< SRef<Decoder_Instance*> >::iterator iCodec;
    SRef<Decoder_Instance*> newCodecInstance;

    for( iCodec = _codecs.begin(); iCodec != _codecs.end(); iCodec ++ )
    {
        if( (*iCodec)->get_sdp_media_type() == payloadType )
            return (*iCodec);
    }

    newCodecInstance = ((Audio_Media *)(*_media))->create_decoder_instance( _realtime_stream, payloadType );
    if( newCodecInstance )
        _codecs.push_back( newCodecInstance );

    return newCodecInstance;
}

std::string Audio_Media_Source::get_decoder_description()
{
    std::string codecDescription;
    for(std::list< SRef<Decoder_Instance*> >::iterator it = _codecs.begin(), end = _codecs.end(); it!=end; ++it)
    {
        SRef<Codec_Description*> codec = (*it)->get_codec();
        if(codec)
            codecDescription += (*it)->get_codec()->get_codec_description() + "; ";
    }
    return codecDescription;
}

uint64_t Audio_Media_Source::get_number_of_received_packets()
{
    return receivedPacketCount;
}

uint64_t Audio_Media_Source::get_number_of_missing_packets()
{
    return lostPacketCount;
}

float Audio_Media_Source::get_received_video_framerate_fps()
{
    return -1.0;
}
