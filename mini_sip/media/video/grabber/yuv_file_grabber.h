#ifndef YUV_FILE_GRABBER_H
#define YUV_FILE_GRABBER_H

#include <stdint.h>
#include <fstream>

#include "mutex.h"
#include "my_semaphore.h"
#include "grabber.h"

class Yuv_File_Grabber : public Grabber
{
public:
    Yuv_File_Grabber(SRef<Video_Media*> _media, std::string deviceId);

    void open();
    void get_capabilities();
    bool set_image_chroma( uint32_t chroma );

    void read();
    virtual void run();
    virtual void stop();
    virtual void start();

    virtual void close();

    uint32_t get_height();
    uint32_t get_width();

    virtual void set_local_display(SRef<Video_Display*>);

private:
    void init();
    void display_local(SRef<SImage*> frame);
    SRef<SImage*> local_rgb;

    std::fstream infile;

    bool do_stop;

    std::string device;

    Mutex grabber_lock;
    SRef<Video_Display*> local_display;

    bool initialized;
    SRef<Thread*> runthread;
    SRef<Semaphore*> start_block_sem;
    SRef<Semaphore*> init_block_sem;
    int fps;
    int deviceno;
    int width, height;
};

class Yuv_File_Plugin : public Grabber_Plugin
{
public:
    Yuv_File_Plugin( SRef<Library *> lib ) : Grabber_Plugin( lib ){}

    virtual SRef<Grabber *> create(SRef<Video_Media*> media, const std::string &device) const;

    virtual std::string get_mem_object_type() const { return "YuvFilePlugin"; }

    virtual std::string get_name() const { return "yuvfile"; }

    virtual uint32_t get_version() const { return 0x00000001; }

    virtual std::string get_description() const { return "Raw yuv420p file grabber"; }
};

#endif // YUV_FILE_GRABBER_H
