#ifndef VIDEO_DISPLAY_H
#define VIDEO_DISPLAY_H

#include "sobject.h"
#include "thread.h"
#include "cond_var.h"
#include "mutex.h"
#include "semaphore.h"
#include "splugin.h"
#include "ssingleton.h"
#include "command_string.h"
#include "message_router.h"

#include "image_handler.h"

#include "media_stream.h"

class Video_Display : public Image_Handler
{
public:
    virtual std::string get_mem_object_type() const { return "VideoDisplay"; }
    virtual ~Video_Display();
    virtual void start() = 0;
    virtual void stop() = 0;

    virtual void set_is_local_video( bool isLocal ) = 0;
    virtual bool get_is_local_video() = 0;

    std::string get_call_id();
    uint32_t get_ssrc();

    virtual int get_displayed_width() = 0;
    virtual int get_displayed_height() = 0;

    virtual void set_media_stream_receiver(SRef<Realtime_Media_Stream_Receiver*> rtMSR);

    void camera_ctrl(int8_t panSpeed, int8_t tiltSpeed, int8_t zoomSpeed, int duration);

protected:
    Video_Display(std::string callId, uint32_t _ssrc);
    std::string call_id;
    uint32_t ssrc;

    virtual void displayed_size_change( int width, int height );
private:
    SRef<Realtime_Media_Stream_Receiver*> rt_media_stream_rec;
};


class Video_Display_Plugin : public SPlugin
{
public:
    virtual std::string get_plugin_type() const;

    Video_Display_Plugin( SRef<Library *> lib );

    virtual SRef<Video_Display *> create( bool fullscreen, std::string callId, uint32_t ssrc) const = 0;
};


class Video_Display_Registry: public SPlugin_Registry, public SSingleton<Video_Display_Registry>
{
public:
    virtual std::string get_plugin_type(){ return "VideoDisplay"; }

    SRef<Video_Display*> create_display( bool doStart, bool fullscreen, std::string callId, uint32_t ssrc);
    void signal_display_deleted();

protected:
    Video_Display_Registry();

private:
    uint32_t display_counter;
    Mutex display_counter_lock;

    friend class SSingleton<Video_Display_Registry>;
};

#endif // VIDEO_DISPLAY_H
