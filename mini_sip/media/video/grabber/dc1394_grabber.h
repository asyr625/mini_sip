#ifndef DC1394_GRABBER_H
#define DC1394_GRABBER_H

#include "mutex.h"
#include "my_semaphore.h"
#include "grabber.h"


#define MAX_PORTS   4
#define MAX_CAMERAS 8

class Dc1394_Grabber : public Grabber
{
public:
    Dc1394_Grabber( SRef<Video_Media*>, uint32_t portId, uint32_t cameraId );

    void open();
    void get_capabilities();
    void print_capabilities();
    void print_image_format();
    void get_image_format();
    bool set_image_chroma( uint32_t chroma );


    void read();
    virtual void run();
    virtual void stop();
    virtual void start();

    virtual void close();

    uint32_t get_height(){ return height; }
    uint32_t get_width(){ return width; }

private:
    dc1394_cameracapture camera;
    raw1394handle_t cameraHandle;
    uint32_t portId;
    uint32_t cameraId;

    uint32_t height;
    uint32_t width;

    bool stopped;

    Mutex grabberLock;
    bool handlerProvidesImage;

    SRef<SImage*> oldImage;
    SRef<Thread*> runthread;
};

class Dc1394_Plugin : public Grabber_Plugin
{
public:
    Dc1394_Plugin( SRef<Library *> lib ) : Grabber_Plugin( lib ){}

    virtual SRef<Grabber *> create( SRef<Video_Media*>, const std::string &device) const;

    virtual std::string get_mem_object_type() const { return "Dc1394Plugin"; }

    virtual std::string get_name() const { return "fw"; }

    virtual uint32_t get_version() const { return 0x00000001; }

    virtual std::string get_description() const { return "Video4linux grabber"; }
};

#endif // DC1394_GRABBER_H
