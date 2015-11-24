#include "video_plugin.h"

using namespace std;

static std::list<std::string> pluginList;
static bool initialized;


extern "C"
std::list<std::string> *mvideo_LTX_listPlugins( SRef<Library*> lib )
{
    if( !initialized )
    {
        pluginList.push_back("getPlugin");
        initialized = true;
    }
    return &pluginList;
}

extern "C"
SPlugin * mvideo_LTX_getPlugin( SRef<Library*> lib )
{
    return new Video_Plugin( lib );
}

Video_Plugin::Video_Plugin(SRef<Library*> lib)
    : Media_Plugin( lib )
{
    Grabber_Registry::get_instance();
    Video_Display_Registry::get_instance();
}

Video_Plugin::~Video_Plugin()
{

}
SRef<Media*> Video_Plugin::create_media( Mini_Sip*minisip, SRef<Sip_Configuration *>config , Streams_Player *)
{
    volatile int *globalBitratePtr = NULL;
#ifdef GLOBAL_BANDWIDTH_HACK
    extern volatile int globalBitRate1;
    globalBitratePtr = &globalBitRate1;
#endif
    SRef<Video_Codec_Description *> videoCodec = new Video_Codec_Description();	//FIXME: use registry instead of creating new description.

    return new Video_Media( minisip, config, *videoCodec, /*streamsPlayer,*/ false );
}

SRef<Media*> Video_Plugin::create_media2stream( Mini_Sip*minisip,  SRef<Sip_Configuration *> config, Streams_Player *)
{
    volatile int *globalBitratePtr = NULL;
#ifdef GLOBAL_BANDWIDTH_HACK
    extern volatile int globalBitRate2;
    globalBitratePtr = &globalBitRate2;
#endif
    SRef<Video_Codec_Description *> videoCodec = new Video_Codec_Description();
    return new Video_Media( minisip, config, *videoCodec, /*streamsPlayer,*/ true );
}

std::string Video_Plugin::get_name() const
{
    return "video";
}

uint32_t Video_Plugin::get_version() const
{
    return 0x00000001;
}

std::string Video_Plugin::get_description() const
{
    return "video media plugin";
}
