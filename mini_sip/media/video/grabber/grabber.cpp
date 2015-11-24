#include "grabber.h"

#include "dbg.h"
#include "video_media.h"
#include "video_exception.h"
#include "codec.h"
#include "video_display.h"
#include "avdecoder.h"


#ifdef HAVE_LINUX_VIDEODEV2_H
#include "v4l2_grabber.h"
#endif
#include "yuv_file_grabber.h"
using namespace std;

Grabber::Grabber(SRef<Video_Media*> _media)
    : media(_media)
{
    processing_video_grabber = new Realtime_Media_Pipeline( media->get_mini_sip(), NULL, NULL , this, "video", PROCESSOR_DIRECTION_DEV2NET, PROCESSOR_INSERT_POINT_DEVICE );
    Processor_Registry::get_instance()->add_plugins( processing_video_grabber, NULL);
}

void Grabber::forward(const SRef<SImage*> _image)
{
    SRef<Processing_Data_Video*> vdata = new Processing_Data_Video;
    vdata->image = _image;
    processing_video_grabber->process(*vdata); //send through pipeline. Output is sent to handleData() below
}

void Grabber::handle_data(const SRef<Processing_Data*>& data)
{
    Processing_Data_Video* vdata = (Processing_Data_Video*)*data;
    media->handle(vdata->image);
}

Grabber_Plugin::Grabber_Plugin( SRef<Library *> lib )
{
}

Grabber_Plugin::~Grabber_Plugin()
{
}

Grabber_Registry::Grabber_Registry()
{
#ifdef HAVE_LINUX_VIDEODEV2_H
    register_plugin( new V4l2_Plugin( NULL ) );
    register_plugin( new Yuv_File_Plugin( NULL ) );
#endif
}

SRef<Grabber*> Grabber_Registry::create_grabber( SRef<Video_Media*> media, std::string device )
{
    SRef<Grabber *> result;
    try
    {
        size_t pos = device.find(':');
        std::string name;
        std::string dev;

        if( pos == std::string::npos )
        {
            name = "v4l2";
            dev = device;
        }
        else
        {
            name = device.substr( 0, pos );
            dev = device.substr( pos + 1 );
        }

        SRef<SPlugin *> plugin = find_plugin( name );

        if( !plugin )
        {
            my_err << "Grabber_Registry: " << name << " grabber not found " << endl;
            return NULL;
        }

        SRef<Grabber_Plugin *> gPlugin = dynamic_cast<Grabber_Plugin *>(*plugin);
        result = gPlugin->create( media, dev );
    }
    catch( Video_Exception & exc )
    {
        my_err << exc.error() << endl;
        return NULL;
    }
    return result;
}
