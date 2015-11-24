#ifndef MEDIA_SHARED_WORKSPACE_H
#define MEDIA_SHARED_WORKSPACE_H

#include <string>
#include "reliable_media.h"

class Media_Shared_Workspace : public Reliable_Media
{
public:
    Media_Shared_Workspace();
    Media_Shared_Workspace(Mini_Sip* minisip);

    virtual std::string get_media_formats() { return "vnc"; }

    virtual SRef<Reliable_Media_Stream*> create_media_stream( std::string callId);

    virtual std::string get_sdp_media_type();

    virtual uint8_t get_codec_get_sdp_media_type();

private:
};

#endif
