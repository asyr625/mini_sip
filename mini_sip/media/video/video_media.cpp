#include "video_media.h"

#include "codec.h"
#include "video_codec.h"
#include "avdecoder.h"
#include "video_exception.h"
#include "grabber.h"
#include "video_display.h"
#include "sdp_headerm.h"
#include "sdp_headera.h"

#include "string_utils.h"
#include "my_assert.h"
#include "session.h"
#include "media_handler.h"
#include "video_stream_player.h"
#include "streams_player.h"

#include<string.h>
#include<stdio.h>
using namespace std;

#define SOURCE_QUEUE_SIZE 7

Video_Media::Video_Media( Mini_Sip*_minisip, SRef<Sip_Configuration *> config, SRef<Codec_Description *> codec, bool isSecondMediaHack)
    : Realtime_Media( _minisip, codec ),
      keyframe_request_callback(NULL)
{
    this->config = config;
    this->codec = dynamic_cast<Video_Codec_Description *>(*codec);
    my_assert( this->codec );

    //NOTE: the "second" device is just a hack in the transition to having multiple input devices per media
    if (isSecondMediaHack)
    {
        grabber = Grabber_Registry::get_instance()->create_grabber( this, config->_video_device2);
        grabber->set_local_display(NULL); //FIXME: no effect - remove?!
    }else{
        grabber = Grabber_Registry::get_instance()->create_grabber( this, config->_video_device); //FIXME: we want to dynamically allocate grabbers, and not a fixed one in the constructor
    }

    receive = true;
    send = !grabber.is_null();

#if 1

    SRef<Video_Display *> localVideoDisplay = Video_Display_Registry::get_instance()->create_display( false, false, "localVideoDisplay",0x0000);
    grabber->set_local_display(localVideoDisplay);
#endif
}

Video_Media::~Video_Media()
{
}

void Video_Media::handle( const SRef<SImage *>& )
{
    SRef<Processing_Data_Video*> vdata = new Processing_Data_Video;
    vdata->image = image;
    send_data_to_streams(*vdata);
}

SRef<Encoder_Instance*> Video_Media::create_encoder_instance( uint8_t payloadType ) //overload default implementation to set default resolution, bandwidth etc.
{
    SRef<Video_Encoder_Instance*> ret = dynamic_cast<Video_Encoder_Instance*>(*Realtime_Media::create_encoder_instance(payloadType));
    my_assert(ret);

    volatile int *globalBitratePtr = NULL;
#ifdef GLOBAL_BANDWIDTH_HACK
    extern volatile int globalBitRate1;
    globalBitratePtr = &globalBitRate1;
#endif

    ret->init_encoder(config->video_encoder1.width, config->video_encoder1.height, config->video_encoder1.framerate, config->video_encoder1.bitrate, globalBitratePtr);
    return *ret;
}


std::string Video_Media::get_sdp_media_type()
{
    return "video";
}

void Video_Media::play_data( const SRef<Rtp_Packet *> & packet )
{
    SRef<Video_Media_Source *> source = get_source( packet->get_header().ssrc );
    my_assert(packet);
    my_assert(source);

    if( source )
    {
        source->play_data( packet );
    }
}

void Video_Media::register_realtime_media_sender( SRef<Realtime_Media_Stream_Sender *> sender )
{
    _senders_lock.lock();
    if( _senders.size() == 0 )
    {
        _senders_lock.unlock();
        try{
            grabber->open();
            if( !grabber->set_image_chroma( M_CHROMA_RV32 ) && !grabber->set_image_chroma( M_CHROMA_I420 ) )
            {
                my_err << "Could not select video capture chroma: " << endl;
                grabber = NULL;
                return;
            }
            grabber->start();
        }
        catch( Video_Exception & exc )
        {
            my_err << "Could not open the video capture device: " << exc.error() << endl;
            grabber = NULL;
        }
        _senders_lock.lock();
    }
    _senders.push_back( sender );
    _senders_lock.unlock();
}

void Video_Media::unregister_realtime_media_sender( SRef<Realtime_Media_Stream_Sender *> sender )
{
    _senders_lock.lock();
    _senders.remove( sender );
    _senders_lock.unlock();

    if( _senders.size() == 0 )
    {
        try{
            grabber->close();
        }
        catch( Video_Exception & exc )
        {
            my_err << "Could not close the video capture device: " << exc.error() << endl;
        }
    }
}

void Video_Media::register_media_source( const SRef<Session*>& session, uint32_t ssrc,
                                         std::string callId, SRef<Realtime_Media_Stream_Receiver*> rtMSR )
{
    SRef<Video_Media_Source *> source;

    source = new Video_Media_Source( session, callId, ssrc, rtMSR);
    source->set_keyframe_request_callback(keyframe_request_callback);

    sources_lock.lock();
    sources.push_back( source );
    sources_lock.unlock();
}

void Video_Media::unregister_media_source( uint32_t ssrc )
{
    SRef<Video_Media_Source *> source = get_source( ssrc );
    if( source )
    {
        sources_lock.lock();
        sources.remove( source );
        sources_lock.unlock();
        source->get_decoder()->close();
        if( source->display )
        {
            source->display->stop();
        }
        //streamsPlayer->remove(source->getVideoStreamPlayer());
    }
}

void Video_Media::handle_mheader( SRef<Sdp_HeaderM *> m )
{

}

uint8_t  Video_Media::get_codec_get_sdp_media_type()
{
    return this->codec->get_sdp_media_type();
}

SRef<Video_Media_Source *> Video_Media::get_source( uint32_t ssrc )
{
    list<SRef<Video_Media_Source *> >::iterator i;

    sources_lock.lock();

    for( i = sources.begin(); i != sources.end(); i++ )
    {
        if( (*i)->ssrc == ssrc )
        {
            sources_lock.unlock();
            return (*i);
        }
    }
    sources_lock.unlock();
    return NULL;
}

std::list<Receiving_MSS_Reporter *> Video_Media::get_receiving_media_sources()
{
    std::list<Receiving_MSS_Reporter *> mediaSources;
    for(std::list< SRef<Video_Media_Source *> >::const_iterator it = sources.begin(), end=sources.end(); it!=end; ++it)
        mediaSources.push_back(**it);
    return mediaSources;
}

void Video_Media::keyframe_request_arrived()
{
    std::list< SRef<Realtime_Media_Stream_Sender *> >::iterator i;
    for (i = _senders.begin(); i != _senders.end();i++)
    {
        (*i)->request_codec_intracoded();
    }
}

Stream_Player* Video_Media::create_video_stream_player(IStream_To_Streams_Player *_owner, Image_Handler *_sink,
                                                       const int &_rtpTimestampSamplingRate,
                                                       const unsigned int &bufferOverflowUnderflowModifier_us,
                                                       const unsigned int &synchronizationToleration_us)
{
    return new Video_Stream_Player(_owner, _sink, _rtpTimestampSamplingRate, bufferOverflowUnderflowModifier_us, synchronizationToleration_us);
}

void Video_Media::set_keyframe_request_callback(IRequest_Video_Keyframe *_keyframeRequestCallback)
{
    sources_lock.lock();
    for(auto it = sources.begin(), end = sources.end(); it != end; ++it)
        (*it)->set_keyframe_request_callback(_keyframeRequestCallback);
    sources_lock.unlock();
    keyframe_request_callback = _keyframeRequestCallback;
}


Video_Media_Source::Video_Media_Source( const SRef<Session*>& session, const std::string &_callID,
                                        const uint32_t &ssrc, SRef<Realtime_Media_Stream_Receiver*> rtMSR )
    : call_id(_callID), ssrc(_ssrc), nbytes_received(0),
      packet_loss(false),
      number_of_received_packets(0), number_of_missing_packets(0)
{
    display = NULL;
    frame_converter = new Threaded_Frame_Converter(this, "Decoder", true, true);
    decoder = new AVDecoder(rtMSR);
    decoder->set_ssrc(ssrc);

    display = Video_Display_Registry::get_instance()->create_display(true, false, callID, ssrc);
//	display->setVideoName(videoName);
    display->set_media_stream_receiver(rtMSR);

    processing_video = new Realtime_Media_Pipeline(session->get_mini_sip(), session, *rtMSR , this, "video", PROCESSOR_DIRECTION_NET2DEV, PROCESSOR_INSERT_POINT_STREAM); //output should be the video stream player - null for now
    Processor_Registry::get_instance()->add_plugins( processing_video, *rtMSR );
}

Video_Media_Source::~Video_Media_Source()
{
}

// 65536 1
int rtpSeqDiff(int prev, int now)
{
    int *largest;
    int *smallest;
    if (prev>now){
        largest= &prev;
        smallest=&now;
    }else{
        largest=&now;
        smallest=&prev;
    }

    int ldiff=*largest - *smallest;

    if (ldiff > 0xFFFF-1000){ //undo wrapping so that later sequence numbers are larger
        *smallest+=0x10000;
    }
    int ret = now-prev;
    return ret;
}

void Video_Media_Source::play_data( const SRef<Rtp_Packet *> & packet )
{
    my_assert(decoder);
    my_assert(packet);
    my_assert((uint64_t)this>1000);

    number_of_received_packets++;
    totalReceivedPacketSize += packet->get_header().size() + packet->get_content_length();

    SRef<Processing_Data*> data =decoder->decode(packet);

    if (data)
    {
        SRef<Processing_Data_Video*> vdata = dynamic_cast<Processing_Data_Video*>(*data);
        frame_converter->handle(vdata->image, vdata->image->width, vdata->image->height, M_CHROMA_RV32, true);//FIXME: disable local copy
    }else{
    }
}

void Video_Media_Source::frame_converter_output(const SRef<SImage*>& image)
{
    SRef<Processing_Data_Video*> vdata = new Processing_Data_Video;
    vdata->image = image;
    processing_video->process(*vdata);
}

SRef<AVDecoder *> Video_Media_Source::get_decoder()
{
    return decoder;
}

std::string Video_Media_Source::get_decoder_description()
{
    SRef<AVDecoder *> decoder = get_decoder();
    if(decoder)
        return decoder->get_description();
    return "";
}

uint64_t Video_Media_Source::get_number_of_received_packets()
{
    return number_of_received_packets;
}

uint64_t Video_Media_Source::get_number_of_missing_packets()
{
    cerr <<"FIXME: number_of_missing_packets counter broken"<<endl;
    return number_of_missing_packets;
}

float Video_Media_Source::get_received_video_framerate_fps()
{
    SRef<AVDecoder *> decoder = get_decoder();
    if(decoder)
        return decoder->get_current_framerate();
    return 0;
}

void Video_Media_Source::set_keyframe_request_callback(IRequest_Video_Keyframe *keyframeRequestCallback)
{
    if(decoder)
        decoder->set_keyframe_request_callback(keyframeRequestCallback);
}

void Video_Media_Source::handle_data(const SRef<Processing_Data*>& data )
{
    SRef<Processing_Data_Video*> vdata = dynamic_cast<Processing_Data_Video*>(*data);
    display->handle(vdata->image);
}

void Video_Media_Source::add_packet_to_frame(const SRef<Rtp_Packet *> & packet, bool flush)
{

}
