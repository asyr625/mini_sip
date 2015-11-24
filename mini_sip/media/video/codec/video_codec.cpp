#include "video_codec.h"

#include "avdecoder.h"
#include "media_handler.h"
#include "codec.h"
#include "video_exception.h"

#include "dbg.h"
using namespace std;

Video_Coder_Factory_Func_Ptr Video_Codec_Description::coder_factory = NULL;

Video_Codec_Description::Video_Codec_Description()
{
    my_assert(coder_factory);
}

std::string Video_Codec_Description::get_codec_name()
{
    return std::string( "H264" );
}

std::string Video_Codec_Description::get_codec_description()
{
    return std::string( "X264 based H.264 Video Encoder/Decoder" );
}

uint8_t Video_Codec_Description::get_sdp_media_type()
{
    return 99;
}

std::string Video_Codec_Description::get_sdp_media_attributes()
{
    return std::string("H264/90000");
}

uint32_t Video_Codec_Description::get_version() const
{
    return 0x00000001;
}

SRef<Decoder_Instance*> Video_Codec_Description::new_decoder_instance(const SRef<Realtime_Media_Stream_Receiver*>&rtmsr)
{
    SRef<Decoder_Instance*> codecState = new AVDecoder(rtmsr);
    codecState->set_codec(this);
    return codecState;
}

SRef<Encoder_Instance*> Video_Codec_Description::new_encoder_instance()
{
    SRef<Video_Encoder_Instance*> codecInstance = coder_factory();
    codecInstance->set_codec(this);
    return *codecInstance;
}


Video_Decoder_Instance::Video_Decoder_Instance(const SRef<Realtime_Media_Stream_Receiver*>& rtmsr)
    : Decoder_Instance(rtmsr)
{
}

Video_Encoder_Instance::Video_Encoder_Instance()
{
}

Video_Codec_Registry::Video_Codec_Registry()
{
    register_plugin( new Video_Codec_Description( ) );
}

SRef<Video_Codec_Description *> Video_Codec_Registry::create( const std::string& codecname)
{
    list< SRef<SPlugin*> >::iterator iter;
    list< SRef<SPlugin*> >::iterator stop = plugins.end();

    for( iter = plugins.begin(); iter != stop; iter++ )
    {
        SRef<SPlugin*> plugin = *iter;

        SRef<Video_Codec_Description*> codec = dynamic_cast<Video_Codec_Description*>(*plugin);

        if( !codec )
        {
            my_err << "Not an VideoCodec? " << plugin->get_name() << endl;
        }

        if( codec && codec->get_codec_name() == codecname )
        {
            return codec;
        }
    }
    my_dbg << "VideoCodec not found name: " << codecname<< endl;
    return NULL;
}
