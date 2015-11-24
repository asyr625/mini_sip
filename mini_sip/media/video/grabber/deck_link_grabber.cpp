#include "deck_link_grabber.h"

#include "video_media.h"
#include "video_exception.h"
#include "my_time.h"
#include "string_utils.h"
#include "my_error.h"
#include "yuv2rgb.h"
#include "threaded_frame_converter.h"

#include<stdio.h>
#include<errno.h>

#include<time.h>

#ifdef LOGGING_SUPPORT
#include "logging_manager.h"
#endif

using namespace std;

static std::list<std::string> pluginList;
static bool initialized;

extern "C"
std::list<std::string> *decklink_LTX_listPlugins( SRef<Library*> lib )
{
    if( !initialized )
    {
        pluginList.push_back("getPlugin");
        initialized = true;
    }
    return &pluginList;
}

extern "C"
SPlugin * decklink_LTX_getPlugin( SRef<Library*> lib )
{
    return new Deck_Link_Plugin( lib );
}


/**
 * Piotr Szymaniak, PSNC: This is like 5 times faster than the sws_scale()
 */
void uyvy_to_yuv420p(int w, int h, byte_t *in, byte_t* outy, byte_t* outu, byte_t* outv)
{
    int x;
    int y;
    int inc;
    int W=w/2;
    for (y=0; y<h; y++)
    {
        inc = y & 1;
        for (x=0; x<W; x++)
        {
            //Take u
            *outu=*in++;
            outu+=inc;

            //Take y1
            *outy++=*in++;

            //Take v
            *outv=*in++;
            outv+=inc;

            //Take y2
            *outy++=*in++;
        }
    }
}


DeckLinkCaptureDelegate::DeckLinkCaptureDelegate()
    : width(0), height(0), frame(NULL), do_stop(false), no_source_flag(true)
{

    allocate_image();
}

void DeckLinkCaptureDelegate::allocate_image()
{
    frame = NULL;
    frame = SImage::factory(width, height, M_CHROMA_I420);
}

HRESULT STDMETHODCALLTYPE DeckLinkCaptureDelegate::video_input_format_changed(BMDVideoInputFormatChangedEvents, IDeckLinkDisplayMode*, BMDDetectedVideoInputFormatFlags)
{
    cerr <<"WARNING: DeckLinkCaptureDelegate::VideoInputFormatChanged called. "<<endl;
    return S_OK;
}
HRESULT STDMETHODCALLTYPE DeckLinkCaptureDelegate::video_input_frame_arrived(IDeckLinkVideoInputFrame*videoFrame, IDeckLinkAudioInputPacket* audioFrame)
{
    void *frameBytes;
    void *audioFrameBytes;
    if(videoFrame)
    {
        if (videoFrame->GetFlags() & bmdFrameHasNoInputSource)
        {
            if(this->no_source_flag == false && filled == false)
            {
                filled = true;
                buffer_sem.inc();
            }
            this->no_source_flag = true;
        }
        else
        {
            this->no_source_flag=false;
            put_image(videoFrame);
        }
        frame_count++;
    }

    // Handle Audio Frame
    if (audioFrame)
    {
    }
    return S_OK;
}

SRef<SImage*> DeckLinkCaptureDelegate::get_image()
{
    buffer_sem.dec();
    filled = false;
    return frame;
}

void DeckLinkCaptureDelegate::put_image(IDeckLinkVideoInputFrame* videoFrame)
{
    int pw = videoFrame->GetWidth();
    int ph = videoFrame->GetHeight();
    byte_t* vbytes;
    videoFrame->GetBytes((void**)&vbytes);

    buffer_lock.lock();
    if(filled)
    {
        buffer_lock.unlock();
        cerr <<"EEEE: Warning: dropping grabbed frame. Low CPU?"<<endl;
        return;
    }
    filled = true;
    if (pw!=width || ph!=height)
    {
        width = pw;
        height = ph;
        allocate_image();
    }
    uyvy_to_yuv420p(pw,ph, vbytes, frame->data[0], frame->data[1], frame->data[2]);
    int64_t unused;
    if(videoFrame->GetStreamTime(&frame->uTime, &unused, 1000000) != S_OK)
    {
        cerr << "DeckLinkCaptureDelegate::putImage() error: frame time acquirement failed, using rtc." << endl;
        frame->uTime = utime();
    }
    buffer_sem.inc();
    buffer_lock.unlock();
}


Deck_Link_Grabber::Deck_Link_Grabber( SRef<Video_Media*>_media, std::string dev )
    : Grabber(_media)
{
    device = dev;
    capture = NULL;
    initialized = false;
}

Deck_Link_Grabber::~Deck_Link_Grabber()
{
    if(local_display)
        local_display->stop();
}

void Deck_Link_Grabber::init()
{
    cerr << "------------------ device=" << device << endl;
    deviceno = 0;
    // Example of format: 0/1080i60  First camera, 1080i, 60FPS
    if (device.size()>0)
        deviceno = device[0]-'0';
    my_assert(deviceno >=0 && deviceno <=9);

    string mode = "720p50";

    if (device.size() > 2)
        mode = device.substr(2);

    int position;
    string fpsString;
    if((position = mode.find('@')) != string::npos)
    {
        cerr << "DeckLinkGrabber::init() warning: @fps no longer supported! Use <video_encoder_format> or <video_encoder_format2> instead." << endl;
        mode = mode.substr(0, position);
    }

    BMDDisplayMode selectedDisplayMode = bmdModeHD720p50;

    bool modeok = false;
    if (mode=="pal")
        selectedDisplayMode = bmdModePAL, modeok=true;

    if (mode=="720p50")
        selectedDisplayMode = bmdModeHD720p50, modeok=true;

    if (mode=="720p5994")
        selectedDisplayMode = bmdModeHD720p5994, modeok=true;

    if (mode=="720p60")
        selectedDisplayMode = bmdModeHD720p60, modeok=true;

    if (mode=="1080i60")
        selectedDisplayMode = bmdModeHD1080i6000, modeok=true;

    if (mode=="1080i5994")
        selectedDisplayMode = bmdModeHD1080i5994, modeok=true;

    if (mode=="1080p60")
        selectedDisplayMode = bmdModeHD1080p6000, modeok=true;

    if (mode=="1080p30")
        selectedDisplayMode = bmdModeHD1080p30, modeok=true;

    if (mode=="1080p2997")
        selectedDisplayMode = bmdModeHD1080p2997, modeok=true;

    if (mode=="1080p25")
        selectedDisplayMode = bmdModeHD1080p25, modeok=true;

    if (mode=="1080i50")
        selectedDisplayMode = bmdModeHD1080i50, modeok=true;

    if (!modeok)
    {
        my_err << "ERROR: Grabber video input format not supported: " << mode << endl;
        my_assert(modeok);
    }

    do_stop = false;
    initialized = true;
    //width=height=-1;
    capture = new DeckLinkCaptureDelegate();

    deck_link_iterator = CreateDeckLinkIteratorInstance();
    HRESULT                     result;

    IDeckLinkDisplayMode        *displayMode;

    if (!deck_link_iterator)
    {
        fprintf(stderr, "This application requires the DeckLink drivers installed.\n");
        initialized = false;
#ifdef LOGGING_SUPPORT
        Logger::get_instance()->info(string("This application requires the DeckLink drivers installed"), "error.decklink");
#endif
        return;
    }

    /* Connect to the first DeckLink instance */
    result = deck_link_iterator->Next(&deck_link);
    if (result != S_OK)
    {
        fprintf(stderr, "No DeckLink PCI cards found.\n");
        initialized = false;
#ifdef LOGGING_SUPPORT
        Logger::get_instance()->info(string("No DeckLink PCI cards found"), "error.decklink");
#endif
        return;
    }
    int d=deviceno;
    while (d>0 && (result = deck_link_iterator->Next(&deck_link)) ==S_OK)
        d--;
    if (result != S_OK)
    {
        fprintf(stderr, "Decklink: Error: Card number %d not found.\n",deviceno);
#ifdef LOGGING_SUPPORT
        Logger::get_instance()->info(string("Decklink card not found"), "error.decklink");
#endif
        my_assert(result==S_OK); //better to quit than continue until it is handled
        initialized = false;
        return;
    }

    if (deck_link->QueryInterface(IID_IDeckLinkInput, (void**)&deck_link_iterator) != S_OK)
    {
        fprintf(stderr, "DeckLink error.\n");
        initialized = false;
        return;
    }

    deck_link_iterator->SetCallback(capture);

    result = deck_link_input->GetDisplayModeIterator(&display_mode_iterator);
    if (result != S_OK)
    {
        fprintf(stderr, "Could not obtain the video output display mode iterator - result = %08x\n", result);
        initialized = false;
#ifdef LOGGING_SUPPORT
        Logger::get_instance()->info(string("Could not obtain the video output display mode iterator"), "error.decklink");
#endif
        return;
    }

    result = deck_link_input->EnableVideoInput(selectedDisplayMode, bmdFormat8BitYUV, 0);
    if(result != S_OK)
    {
        fprintf(stderr, "Failed to enable video input. Is another application using the card?\n");
        initialized = false;
#ifdef LOGGING_SUPPORT
        Logger::get_instance()->info(string("Failed to enable video input. Another application may be using the card"), "error.decklink");
#endif
        return;
    }
}

void Deck_Link_Grabber::open()
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

void Deck_Link_Grabber::get_capabilities()
{
}

bool Deck_Link_Grabber::set_image_chroma( uint32_t chroma )
{
    cerr << "Warning: ignoring setImageChroma request for chroma "<< chroma <<endl;
    return true;
}

void Deck_Link_Grabber::read()
{
    if (local_display)
    {
        local_display->set_is_local_video(true);
        local_display->start();
    }
    HRESULT result = deck_link_input->StartStreams();
    if(result != S_OK)
    {
        cerr<<"ERROR: could not start DeckLink capturing"<<endl;
#ifdef LOGGING_SUPPORT
        Logger::get_instance()->info(string("Could not start DeckLink capturing"), "error.decklink");
#endif
        return;
    }

    while (!do_stop)
    {
        static int i = 0;
        i++;
        if(capture->get_no_source_flag() == true)
        {
            cerr<<"No Input Signal received"<<endl;
            my_sleep(100);
            continue;
        }
        SRef<SImage*> frame = capture->get_image();
        forward( frame );
        if (local_display && i%2==0)
        {
            SRef<SImage*> framergb = SImage::factory(frame->width, frame->height, M_CHROMA_RV24);
            yuv2rgb(&frame->data[0][0], frame->linesize[0],
                    &frame->data[1][0],frame->linesize[1],
                    &frame->data[2][0],frame->linesize[2],
                    frame->width,
                    frame->height,
                    &framergb->data[0][0] );

            local_display->handle(framergb/*, frame->width, frame->height, M_CHROMA_RV24, true*/);
        }

#ifdef DEBUG_OUTPUT
        if (i%200==0)
        {
            static struct timespec last_wallt;
            static struct timespec last_cput;

            struct timespec now_cpu;
            struct timespec now_wall;
            clock_gettime(CLOCK_THREAD_CPUTIME_ID, &now_cpu);
            clock_gettime(CLOCK_REALTIME, &now_wall);
            cerr <<"cpusec="<<now_cpu.tv_sec<<" lastsec="<<last_cput.tv_sec<<endl;
            uint64_t delta_cpu = (now_cpu.tv_sec-last_cput.tv_sec)*1000000000LL+(now_cpu.tv_nsec-last_cput.tv_nsec);
            uint64_t delta_wall = (now_wall.tv_sec-last_wallt.tv_sec)*1000000000LL+(now_wall.tv_nsec-last_wallt.tv_nsec);

            cerr <<"DeckLinkGrabber:: CPU USAGE: "<< now_cpu.tv_sec<<"."<<now_cpu.tv_nsec<<endl;
            cerr <<"delta_cpu="<<delta_cpu/1000<<" delta_wall="<<delta_wall/1000<<endl;
            cerr <<"========> DeckLinkGrabber: CPU usage: "<< ((float)delta_cpu/(float)delta_wall)*100.0<<"%"<<endl;
            char temp[100];
            sprintf(temp, "%.2f", ((float)delta_cpu/(float)delta_wall)*100.0);
#ifdef LOGGING_SUPPORT
            Logger::get_instance()->info(temp, "info.grabbercpu");
#endif
            last_cput = now_cpu;
            last_wallt = now_wall;

        }
#endif
    }
    result = deck_link_input->StopStreams();
    deck_link_input->FlushStreams();
    Thread::msleep(100);
    if (local_display)
    {
        local_display->stop();
    }

    if (display_mode_iterator != NULL)
    {
        display_mode_iterator->Release();
        display_mode_iterator = NULL;
    }

    if (deck_link_input != NULL)
    {
        deck_link_input->Release();
        deck_link_input = NULL;
    }

    if (deck_link != NULL)
    {
        deck_link->Release();
        deck_link = NULL;
    }

    if (deck_link_iterator != NULL)
        deck_link_iterator->Release();
    initialized = false;
}

void Deck_Link_Grabber::run()
{
#ifdef DEBUG_OUTPUT
    set_thread_name("DeckLinkGrabber::run");
#endif
    //	UserStruct.stopped = false;
    do_stop = false;

    if (!initialized)
        init();

    init_block_sem->inc(); // tell thread blocking in "open()" to continue.

    start_block_sem->dec(); // wait until start() has been called
    start_block_sem = NULL; // free
    read();
}

void Deck_Link_Grabber::start()
{
    do_stop = false;
    if (capture)
        capture->do_stop = false;
    my_assert(start_block_sem);
    start_block_sem->inc();
#ifdef LOGGING_SUPPORT
    Logger::get_instance()->info(string("Decklink grabber started"), "info.decklink");
#endif
}

void Deck_Link_Grabber::stop()
{
    do_stop = true;
    if (capture){
        capture->stop();
    }
    if (runthread){
        runthread->join();
        runthread=NULL;
    }
#ifdef LOGGING_SUPPORT
    Logger::get_instance()->info(string("Decklink grabber stopped"), "info.decklink");
#endif
}

void Deck_Link_Grabber::close()
{
    stop();
}

uint32_t Deck_Link_Grabber::get_height()
{
    if (capture)
    {
        return capture->get_height();
    }
    else
        return 0;
}

uint32_t Deck_Link_Grabber::get_width()
{
    if (capture)
    {
        return capture->get_width();
    }
    else
        return 0;
}

void Deck_Link_Grabber::set_local_display(SRef<Video_Display*>d)
{
    local_display = d;
}


SRef<Grabber *> Deck_Link_Plugin::create( SRef<Video_Media*>, const std::string &device) const
{
    return new Deck_Link_Grabber( media, device );
}
