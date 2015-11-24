#ifndef SUBSYSTEM_MEDIA_H
#define SUBSYSTEM_MEDIA_H

#include "message_router.h"
#include "sip_configuration.h"
class Sip_Identity;
class Mini_Sip;
class Session;
class Session_Registry;

class Ip_Provider;

class Subsystem_Media : public Command_Receiver
{
public:
    Subsystem_Media( Mini_Sip* minisip,
            SRef<Sip_Configuration *> config,
            SRef<Ip_Provider *> ipProvider,
            SRef<Ip_Provider *> ip6Provider = NULL );

    virtual ~Subsystem_Media();

    void handle_command(std::string subsystem, const Command_String & command );

    Command_String handle_command_resp(std::string subsystem, const Command_String& command);

    void set_message_router_callback(SRef<Command_Receiver*> callback);

    SRef<Session *> create_session( SRef<Sip_Identity*> ident, std::string callId );

    virtual std::string get_mem_object_type() const {return "SubsystemMedia";}

    SRef<Session_Registry*> get_session_registry();
private:
    void *_media_handler;
};

#endif // SUBSYSTEM_MEDIA_H
