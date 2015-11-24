#ifndef SWSPLUGIN_H
#define SWSPLUGIN_H

#include  "library.h"
#include  "media.h"

#include "media_shared_workspace.h"

class Shared_Workspace_Plugin : public Media_Plugin
{
public:
    Shared_Workspace_Plugin(SRef<Library*> lib);
    ~Shared_Workspace_Plugin();

    virtual SRef<Media*> create_media( Mini_Sip*, SRef<Sip_Configuration *> config, Streams_Player *streamsPlayer );
    virtual SRef<Media*> create_media2stream ( Mini_Sip*, SRef<Sip_Configuration *> config, Streams_Player *streamsPlayer );
    virtual std::string get_mem_object_type() const { return "SharedWorkspacePlugin"; }
    virtual std::string get_name() const;
    virtual uint32_t get_version() const;
    virtual std::string get_description() const;

private:
};

#endif
