#include "video_display.h"
#include "dbg.h"
#include "video_display.h"
#include "video_exception.h"
#include "rtcp_packet.h"
#include "rtcp_report_app_camctrl.h"

#include<iostream>
#define NB_IMAGES 3

using namespace std;

Video_Display::Video_Display(std::string callId, uint32_t _ssrc)
    : call_id(callId), ssrc(_ssrc)
{
}

Video_Display::~Video_Display()
{
    Video_Display_Registry::get_instance()->signal_display_deleted();
}

std::string Video_Display::get_call_id()
{
    return call_id;
}

uint32_t Video_Display::get_ssrc()
{
    return ssrc;
}

void Video_Display::set_media_stream_receiver(SRef<Realtime_Media_Stream_Receiver*> rtMSR)
{
    rt_media_stream_rec = rtMSR;
}

void Video_Display::displayed_size_change( int width, int height )
{
    if(( width + width % 2) == (height + height % 2) )
    {
        cerr<<"BUG WARNING: same witdht and height. We do not notify. "<<width<<"x"<< height<<endl;
        return;
    }

    if(!this->get_is_local_video() && rt_media_stream_rec)
    {
        rt_media_stream_rec->send_rtcp_app_view( 0, ssrc, width, height );
    }
    else
    {
        if (!this->get_is_local_video())
            cerr <<"WARNING: VideoDisplay::displayedSizeChange: rtMediaStreamRec is NULL"<<endl;
    }
}

void Video_Display::camera_ctrl(int8_t panSpeed, int8_t tiltSpeed, int8_t zoomSpeed, int duration)
{
    SRef<Rtcp_Packet*> pkt = new Rtcp_Packet;
    pkt->add_report( new Rtcp_Report_App_Camctrl(0, ssrc, panSpeed, tiltSpeed, zoomSpeed, duration) );
    if (rt_media_stream_rec)
        rt_media_stream_rec->send_rtcp_packet(pkt);
    else
    {
        my_err <<"WARNING: could not send camera control packet - no RTMSR"<<endl;
    }
}

Video_Display_Plugin::Video_Display_Plugin( SRef<Library *> lib ): SPlugin( lib )
{
}

std::string Video_Display_Plugin::get_plugin_type() const
{
    return "Video_Display";
}
Video_Display_Registry::Video_Display_Registry()
    : display_counter(0)
{
}

SRef<Video_Display*> Video_Display_Registry::create_display( bool doStart, bool fullscreen, std::string callId, uint32_t ssrc)
{
    SRef<Video_Display *> display = NULL;
    const char names[3][16] = {"internal", "filedisplay", "" };
    //this->config = config;
    display_counter_lock.lock();
    for( int i = 0; names[i][0]; i++ )
    {
        string name = names[i];

        if( display_counter != 0 && ( name == "sdl" || name == "xv" ) )
            continue;

        try
        {
            SRef<SPlugin *> plugin;
            plugin = find_plugin( name );
            if( !plugin )
            {
                my_dbg << "Video_DisplayRegistry: Can't find " << name << endl;
                continue;
            }

            SRef<Video_Display_Plugin *> videoPlugin;

            videoPlugin = dynamic_cast<Video_Display_Plugin*>( *plugin );
            if( !videoPlugin )
            {
                my_err << "Video_Display_Plugin: Not display plugin " << name << endl;
                continue;
            }
            display = videoPlugin->create( fullscreen, callId, ssrc );
            if( !display )
            {
                my_err << "Video_Display_Plugin: Couldn't create display " << name << endl;
                continue;
            }
            if (doStart)
                display->start();
            display_counter ++;
            break;
        }
        catch( Video_Exception & exc )
        {
            my_dbg << "Error opening the video display: "
                   << exc.error() << endl;
        }
    }
    display_counter_lock.unlock();
    return display;
}

void Video_Display_Registry::signal_display_deleted()
{
    display_counter_lock.lock();
    display_counter --;
    display_counter_lock.unlock();
}
