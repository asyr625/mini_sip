#include<stdio.h>
#include<errno.h>
#include<time.h>

#include "yuv_file_grabber.h"
#include "video_media.h"
#include "video_exception.h"

#include "my_time.h"
#include "my_error.h"
#include "yuv2rgb.h"
#include "string_utils.h"

#ifdef LOGGING_SUPPORT
#include "logging_manager.h"
#endif

using namespace std;

static std::list<std::string> pluginList;
static bool initialized;

extern "C"
std::list<std::string> *yuvfile_LTX_listPlugins( SRef<Library*> lib )
{
    if( !initialized )
    {
        pluginList.push_back("getPlugin");
        initialized = true;
    }

    return &pluginList;
}

extern "C"
SPlugin * yuvfile_LTX_getPlugin( SRef<Library*> lib )
{
    return new Yuv_File_Plugin( lib );
}

Yuv_File_Grabber::Yuv_File_Grabber( SRef<Video_Media*> _media, std::string deviceId)
    : Grabber(_media),
      device(deviceId),
      initialized(false)
{
}

void Yuv_File_Grabber::set_local_display(SRef<Video_Display*> p)
{
    local_display = p;
}

uint32_t Yuv_File_Grabber::get_height()
{
    return height;
}

uint32_t Yuv_File_Grabber::get_width()
{
    return width;
}

void Yuv_File_Grabber::init()
{
    deviceno = 0;
    // Example of format: /path/to/file/video.yuv,@30,w=1920,h=1080
    std::vector<string> parts = split(device,true,',');
    string filename = parts[0];
    fps = 30;
    width = 1920;
    height = 1080;
    for (int i = 1; i < parts.size(); i++)
    {
        string fpsstr;
        string wstr;
        string hstr;
        switch (parts[i][0])
        {
        case '@':
            fpsstr = parts[i].substr(1);
            fps = atoi(fpsstr.c_str());
            break;
        case 'w':
            my_assert(parts[i][1]=='=');
            wstr = parts[i].substr(2);
            width = atoi(wstr.c_str());
            break;
        case 'h':
            my_assert(parts[i][1]=='=');
            hstr = parts[i].substr(2);
            height = atoi(hstr.c_str());
            break;
        default:
            break;
        }
    }

    infile.open(filename.c_str());
    my_assert( !infile.bad() );
}

void Yuv_File_Grabber::open()
{
    my_assert(!start_block_sem);
    my_assert(!runthread); // we don't want to start a second thread
    // for a grabber object
    start_block_sem = new Semaphore();
    init_block_sem = new Semaphore();
    runthread = new Thread(this, Thread::Normal_Priority);

    init_block_sem->dec(); // wait for init to complete
    init_block_sem = NULL;
}

void Yuv_File_Grabber::close()
{
    stop();
}

bool Yuv_File_Grabber::set_image_chroma( uint32_t chroma )
{
    cerr << "Warning: ignoring setImageChroma request for chroma "<< chroma <<endl;
    return true;
}

void Yuv_File_Grabber::stop()
{
    do_stop = true;
    if (runthread)
    {
        runthread->join();
        runthread=NULL;
    }
}

void Yuv_File_Grabber::start()
{
    do_stop = false;
    my_assert(start_block_sem);
    start_block_sem->inc();
}

void Yuv_File_Grabber::get_capabilities()
{

}

void Yuv_File_Grabber::read()
{
    if (local_display)
    {
        local_display->set_is_local_video(true);
        local_display->start();
    }
    SRef<SImage *> frame = SImage::factory(width, height, M_CHROMA_I420);

    while (!do_stop)
    {
        static int i = 0;
        if (!infile.eof())
            infile.read((char*)frame->data[0], width*height);
        if (!infile.eof())
            infile.read((char*)frame->data[1], width*height / 4);
        if (!infile.eof())
            infile.read((char*)frame->data[2], width*height / 4);

        if (infile.eof() || infile.bad() )
        {
            infile.clear();
            infile.seekg(0, ios::beg);
        }
        else
        {
            my_sleep(1000/fps);
            frame->uTime = utime();
            forward(frame);
            display_local(frame);
        }

    }

    Thread::msleep(100);
    if (local_display)
    {
        local_display->stop();
    }

    initialized = false;
}

void Yuv_File_Grabber::run()
{
#ifdef DEBUG_OUTPUT
    set_thread_name("Yuv_File_Grabber::run");
#endif
    //	UserStruct.stopped = false;
    do_stop = false;

    if (!initialized)
        init();

    init_block_sem->inc(); // tell thread blocking in "open()" to continue.


    init_block_sem->dec(); // wait until start() has been called
    start_block_sem = NULL; // free
    read();

    infile.close();
}

void Yuv_File_Grabber::display_local(SRef<SImage*> frame)
{
    if (deviceno != 0)
        return;

#define SCALE 1
    int targetWidth = frame->width/SCALE;
    int targetHeight = frame->height/SCALE;

    if (!local_rgb || local_rgb->width != targetWidth || local_rgb->height != targetHeight)
    {
        //MImage::destroy(localRgb);
        local_rgb = NULL;
        local_rgb = SImage::factory(targetWidth, targetHeight, M_CHROMA_RV24);
    }

    yuv2rgb( &frame->data[0][0], frame->linesize[0],
            &frame->data[1][0],frame->linesize[1],
            &frame->data[2][0],frame->linesize[2],
            frame->width,
            frame->height,
            &local_rgb->data[0][0]
            );

    if (local_display)
    {
        local_display->handle(local_rgb);
    }
}


SRef<Grabber *> Yuv_File_Plugin::create( SRef<Video_Media*>media, const std::string &device) const
{
    return new Yuv_File_Grabber( media, device );
}
