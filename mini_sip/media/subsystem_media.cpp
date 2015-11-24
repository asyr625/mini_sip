#include "subsystem_media.h"
#include "media_handler.h"

#define MH (*(SRef<Media_Handler*> *) _media_handler)

Subsystem_Media::Subsystem_Media( Mini_Sip* minisip,
        SRef<Sip_Configuration *> config,
        SRef<Ip_Provider *> ipProvider,
        SRef<Ip_Provider *> ip6Provider )
{
    Media_Handler *mh = new Media_Handler(minisip, config, ipProvider, ip6Provider);
    _media_handler = new SRef<Media_Handler*>(mh);
}

Subsystem_Media::~Subsystem_Media()
{
    if( _media_handler )
    {
        SRef<Media_Handler*> * mrefPtr = (SRef<Media_Handler*> *) _media_handler;
        delete mrefPtr;
        _media_handler = NULL;
    }
}


Command_String Subsystem_Media::handle_command_resp(std::string subsystem, const Command_String& command)
{
    assert(subsystem == "media");
    return MH->handle_command_resp(subsystem, command);
}

void Subsystem_Media::set_message_router_callback(SRef<Command_Receiver*> callback)
{
    MH->set_message_router_callback(callback);
}

void Subsystem_Media::handle_command(std::string subsystem, const Command_String & command )
{
    assert(subsystem == "media");
    MH->handle_command(subsystem, command);
}

SRef<Session *> Subsystem_Media::create_session( SRef<Sip_Identity*> ident, std::string callId )
{
    return MH->create_session(ident, callId);
}

SRef<Session_Registry*> Subsystem_Media::get_session_registry()
{
    SRef<Media_Handler*> mh = *MH;
    return *mh;
}
