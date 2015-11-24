#ifndef GRABBER_H
#define GRABBER_H

#include "sobject.h"
#include "splugin.h"
#include "thread.h"
#include "ssingleton.h"

#include "media_processor.h"
#include "video_display.h"

class Video_Media;

class Grabber : public Runnable, public Media_Pipeline_Output_Handler
{
public:
    Grabber(SRef<Video_Media*> _media);
    virtual void open() = 0;

    virtual bool set_image_chroma( uint32_t chroma ) = 0;

    virtual void read() = 0;
    virtual void run() = 0;

    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void close() = 0;

    void forward(const SRef<SImage*> _image);

    virtual void set_local_display(SRef<Video_Display*>) = 0;

    virtual std::string get_mem_object_type() const {return "Grabber";}
protected:
    Mutex lock;
    SRef<Video_Media*> media;
private:
    virtual void handle_data(const SRef<Processing_Data*>& data);
    SRef<Realtime_Media_Pipeline*> processing_video_grabber;
};

class Grabber_Plugin : public SPlugin
{
public:
    Grabber_Plugin( SRef<Library *> lib );
    virtual ~Grabber_Plugin();

    virtual std::string get_plugin_type() const
    {
        return "Grabber";
    }

    virtual SRef<Grabber *> create( SRef<Video_Media*>, const std::string &device ) const = 0;
};

class Grabber_Registry: public SPlugin_Registry, public SSingleton<Grabber_Registry>
{
public:
    virtual std::string get_plugin_type(){ return "Grabber"; }

    SRef<Grabber*> create_grabber(SRef<Video_Media*> media, std::string deviceName );

protected:
    Grabber_Registry();

    friend class SSingleton<Grabber_Registry>;
};

#include "video_media.h"

#endif // GRABBER_H
