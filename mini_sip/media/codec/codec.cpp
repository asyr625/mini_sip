#include "codec.h"
#include "my_assert.h"
#include "dbg.h"

#include <list>
using namespace std;

Encoder_Instance::Encoder_Instance()
    : _rtp_payload_type_no(255)
{
}

void Encoder_Instance::set_rtp_payload_type_no(uint8_t pt)
{
    _rtp_payload_type_no = pt;
}

uint8_t Encoder_Instance::get_rtp_payload_type_no()
{
    return _rtp_payload_type_no;
}


Decoder_Instance::Decoder_Instance(const SRef<Realtime_Media_Stream_Receiver*>& rtmsr)
    : realtimeStream(rtmsr)
{
}

Audio_Codec_Registry::Audio_Codec_Registry()
{
    register_plugin( new G711_Codec_Description( NULL, G711U ) );
    register_plugin( new G711_Codec_Description( NULL, G711A ) );
}

SRef<Decoder_Instance*> Audio_Codec_Registry::create_decoder_instance(const SRef<Realtime_Media_Stream_Receiver*>&rtmsr, uint8_t payloadType )
{
    list< SRef<SPlugin*> >::iterator iter;
    list< SRef<SPlugin*> >::iterator stop = plugins.end();

    for( iter = plugins.begin(); iter != stop; iter++ )
    {
        SRef<SPlugin*> plugin = *iter;

        SRef<Audio_Codec_Description*> codec = dynamic_cast<Audio_Codec_Description*>(*plugin);

        if( !codec )
        {
            my_err << "Not an AudioCodec? " << plugin->get_name() << endl;
        }

        if( codec && codec->get_sdp_media_type() == payloadType )
        {
            return codec->new_decoder_instance(rtmsr);
        }
    }

    my_err << "AudioCodec not found pt: " << payloadType << endl;
    return NULL;
}

SRef<Encoder_Instance*> Audio_Codec_Registry::create_encoder_instance( uint8_t payloadType )
{
    list< SRef<SPlugin*> >::iterator iter;
    list< SRef<SPlugin*> >::iterator stop = plugins.end();

    for( iter = plugins.begin(); iter != stop; iter++ )
    {
        SRef<SPlugin*> plugin = *iter;

        SRef<Audio_Codec_Description*> codec = dynamic_cast<Audio_Codec_Description*>(*plugin);

        if( !codec )
        {
            my_err << "Not an AudioCodec? " << plugin->get_name() << endl;
        }

        if( codec && codec->get_sdp_media_type() == payloadType )
        {
            return codec->new_encoder_instance();
        }
    }

    my_err << "AudioCodec not found pt: " << payloadType << endl;
    return NULL;
}

SRef<Audio_Codec_Description *> Audio_Codec_Registry::create( const std::string& description)
{
    list< SRef<SPlugin*> >::iterator iter;
    list< SRef<SPlugin*> >::iterator stop = plugins.end();

    for( iter = plugins.begin(); iter != stop; iter++ )
    {
        SRef<SPlugin*> plugin = *iter;

        SRef<Audio_Codec_Description*> codec = dynamic_cast<Audio_Codec_Description*>(*plugin);

        if( !codec )
        {
            my_err << "Not an AudioCodec? " << plugin->get_name() << endl;
        }

        if( codec && codec->get_codec_name() == description )
        {
            return codec;
        }
    }

    my_dbg << "AudioCodec not found name: " << description << endl;
    return NULL;
}
