#ifndef V4L2_GRABBER_H
#define V4L2_GRABBER_H


#include<string>
#include<stdint.h>

#include<linux/types.h>
#include<linux/videodev2.h>

#include "mutex.h"
#include "grabber.h"
#include "threaded_frame_converter.h"

#define N_BUFFERS 20

struct v4l2_capability;

struct cardBuffer
{
    uint8_t * start;
    size_t length;
};


typedef enum
{
    IO_METHOD_READ,
    IO_METHOD_MMAP,
    IO_METHOD_USERPTR,
} io_method;

class V4l2_Grabber : public Grabber
{
public:
    V4l2_Grabber(SRef<Video_Media*>, std::string _device );

    virtual void open();
    bool set_image_chroma( uint32_t chroma );
    virtual void read();
    virtual void run();

    virtual void start();
    virtual void stop();

    virtual void close();
    virtual void set_local_display(SRef<Video_Display*>);
    virtual void init();
    virtual void uninit();
    virtual void map_memory();
    virtual void unmap_memory();
    bool set_image_size( uint32_t width, uint32_t height );

private:
    std::string device; //device name as specified by user
    int requested_width; //capture width as specified by user
    int requested_height; //capture height as specified by user
    int requested_fps;    //capture fps as specified by user

    bool is_mjpeg;	//if capturing MJPEG

    int fd;
    struct cardBuffer *         buffers;
    int nframes;
    int n_buffers;

    int grab_width;
    int grab_height;

    Mutex grabber_lock;

    bool stopped;
    SRef<Thread*> runthread;
    SRef<Video_Display*> local_display;
};

class V4l2_Plugin : public Grabber_Plugin
{
public:
    V4l2_Plugin( SRef<Library *> lib ) : Grabber_Plugin( lib ){}

    virtual SRef<Grabber *> create( SRef<Video_Media*> media, const std::string &device) const
    {
        return new V4l2_Grabber( media, device );
    }

    virtual std::string get_mem_object_type() const { return "V4l2Plugin"; }

    virtual std::string get_name() const { return "v4l2"; }

    virtual uint32_t get_version() const { return 0x00000001; }

    virtual std::string get_description() const { return "Video4linux2 grabber"; }
};

#endif // V4L2_GRABBER_H
