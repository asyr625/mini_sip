#ifndef VIDEO_PLUGIN_H
#define VIDEO_PLUGIN_H

#include "sip_configuration.h"
#include "media.h"
#include "video_media.h"
#include "grabber.h"
#include "video_codec.h"

#include "media_stream.h"

class Video_Plugin : public Media_Plugin
{
public:
    Video_Plugin(SRef<Library*> lib);
    virtual ~Video_Plugin();

    virtual SRef<Media*> create_media( Mini_Sip*, SRef<Sip_Configuration *> , Streams_Player *);
    virtual SRef<Media*> create_media2stream(Mini_Sip*,  SRef<Sip_Configuration *> config, Streams_Player *);

    virtual std::string get_mem_object_type() const{ return "VideoPlugin"; }
    virtual std::string get_name() const;
    virtual uint32_t get_version() const;
    virtual std::string get_description() const;
};

#endif // VIDEO_PLUGIN_H
