#include <list>
#include <string>
#include <iostream>
using namespace std;

#include "library.h"
#include "media.h"
#include "media_shared_workspace_plugin.h"
#include "media_shared_workspace.h"

static std::list<std::string> pluginList;

static bool initialized;

extern "C"
std::list<std::string> *msws_LTX_listPlugins( SRef<Library*> lib )
{
    if( !initialized )
    {
		pluginList.push_back("getPlugin");
		initialized = true;
	}
	return &pluginList;
}

extern "C"
SPlugin * msws_LTX_getPlugin( SRef<Library*> lib )
{
    return (SPlugin*)new Shared_Workspace_Plugin( lib );
}

Shared_Workspace_Plugin::Shared_Workspace_Plugin(SRef<Library *> lib )
    : Media_Plugin( lib )
{
}

Shared_Workspace_Plugin::~Shared_Workspace_Plugin()
{
}

SRef<Media_Shared_Workspace*> swsm;

SRef<Media*> Shared_Workspace_Plugin::create_media(Mini_Sip* minisip, SRef<Sip_Configuration *> config, Streams_Player * )
{
    SRef<Media_Shared_Workspace*> swsMedia = new Media_Shared_Workspace(minisip);
	swsm = swsMedia;
	return *swsMedia;
}

SRef<Media*> Shared_Workspace_Plugin::create_media2stream ( Mini_Sip*, SRef<Sip_Configuration *> config, Streams_Player *streamsPlayer )
{
    return NULL;
}

std::string Shared_Workspace_Plugin::get_name() const
{
	return "sharedworkspace";
}

uint32_t Shared_Workspace_Plugin::get_version() const
{
	return 0x00000001;
}

std::string Shared_Workspace_Plugin::get_description() const
{
	return "presentation sharing plugin";
}
