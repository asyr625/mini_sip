#ifndef AUDIO_PLUGIN_H
#define AUDIO_PLUGIN_H

#include "media.h"
#include "sip_configuration.h"

class Audio_Plugin : public Media_Plugin
{
public:
    Audio_Plugin(SRef<Library*> lib);
    virtual ~Audio_Plugin();

    virtual SRef<Media*> create_media(Mini_Sip*minisip, SRef<Sip_Configuration *> config, Streams_Player *streamsPlayer );
    virtual SRef<Media*> create_media2stream ( Mini_Sip*, SRef<Sip_Configuration *> config, Streams_Player *streamsPlayer )
    {
        return NULL;
    }

    virtual std::string get_mem_object_type() const { return "AudioPlugin"; }
    virtual std::string get_name() const{ return "audio"; }
    virtual uint32_t get_version() const{ return 0x00000001; }
    virtual std::string get_description() const { return "audio media plugin"; }
};

#endif // AUDIO_PLUGIN_H
