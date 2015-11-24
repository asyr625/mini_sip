#include "audio_plugin.h"
#include "audio_media.h"
#include "sound_card/sound_device.h"
#include "audio_defines.h"

using namespace std;

static std::list<std::string> pluginList;
static int initialized;
static SRef<SPlugin *> plugin;


extern "C"
std::list<std::string> *maudio_ltx_list_plugins( SRef<Library*> lib )
{
    if( !initialized )
    {
        pluginList.push_back("getPlugin");

        plugin = new Audio_Plugin( lib );
    }

    return &pluginList;
}

extern "C"
SPlugin * maudio_ltx_get_plugin( SRef<Library*> lib )
{
    return *plugin;
}


Audio_Plugin::Audio_Plugin( SRef<Library*> lib ): Media_Plugin( lib )
{
}

Audio_Plugin::~Audio_Plugin()
{
}

SRef<Media*> Audio_Plugin::create_media( Mini_Sip* minisip, SRef<Sip_Configuration *> config, Streams_Player *streamsPlayer )
{
    std::string soundDevIn = config->_sound_device_in;
    std::string soundDevOut = config->_sound_device_out;
    if( soundDevIn == "" || soundDevOut == "")
        return NULL;

    SRef<Sound_Device *> sounddevin = Sound_Device::create( soundDevIn );
    SRef<Sound_Device *> sounddevout;

    if( soundDevIn == soundDevOut )
        sounddevout = sounddevin;
    else
        sounddevout = Sound_Device::create( soundDevOut );

    SRef<Sound_IO *> soundIo = new Sound_IO( sounddevin, sounddevout,
                           config->_sound_iomixer_type,
                           2,  //number of channels
                           SOUND_CARD_FREQ ); //sampling rate

    std::list<SRef<Codec_Description *> > codecList;
    std::list<std::string>::iterator iCodec;

    for( iCodec = config->_audio_codecs.begin(); iCodec != config->_audio_codecs.end(); iCodec ++ )
    {
        SRef<Codec_Description *> selectedCodec;
        SRef<Audio_Codec_Description *> codec = Audio_Codec_Registry::get_instance()->create( *iCodec );


        if( codec )
        {
            selectedCodec = *codec;
        }

        if( selectedCodec )
        {
            my_dbg << "Adding audio codec: " << selectedCodec->get_codec_name() << endl;
            codecList.push_back( selectedCodec );
        }
    }
    return new Audio_Media( minisip, soundIo, codecList, streamsPlayer );
}
