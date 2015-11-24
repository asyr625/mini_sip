#include "media.h"
#include "audio_plugin.h"

Media::Media(Mini_Sip* _minisip)
    : _mini_sip(_minisip)
{
}

Media::~Media()
{
}

std::list<std::string> Media::get_sdp_attributes()
{
    return _sdp_attributes;
}

void Media::add_sdp_attribute( std::string attribute )
{
    _sdp_attributes.push_back( attribute );
}


Mini_Sip* Media::get_mini_sip()
{
    return _mini_sip;
}

void Media::handle_mheader( SRef<Sdp_HeaderM *> m )
{
}


Media_Plugin::Media_Plugin(SRef<Library *> lib)
    : SPlugin( lib )
{
}

Media_Plugin::~Media_Plugin()
{
}

Media_Registry::Media_Registry()
{
    register_plugin( new Audio_Plugin( NULL ) );
#ifdef VNC_SUPPORT
    register_plugin( new Shared_Workspace_Plugin( NULL ) );
#endif
}
