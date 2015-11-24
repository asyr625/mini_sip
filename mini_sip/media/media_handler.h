#ifndef MEDIA_HANDLER_H
#define MEDIA_HANDLER_H

#include "sobject.h"
#include "media.h"
#include "audio_media.h"
#include "session_registry.h"
#include "message_router.h"


class Media_Handler : public virtual SObject, public Session_Registry, public Command_Receiver
{
public:
    Media_Handler( Mini_Sip* minisip,  SRef<Sip_Configuration *> config, SRef<Ip_Provider *> ipProvider, SRef<Ip_Provider *> ip6Provider = NULL );
    virtual ~Media_Handler();

    SRef<Session *> create_session(SRef<Sip_Identity*> id, std::string callId );

    void add_session ( SRef<Session *> session );
    void delete_session ( std::string callId );
    void parse_screen_command(char * buf, char **args);

    SRef<Session *> get_session (std::string callId);

    void register_media( SRef<Media *> media );

    void handle_command(std::string subsystem, const Command_String & command );

    Command_String handle_command_resp(std::string subsystem, const Command_String& command);

    std::string get_ext_ip();

    void set_message_router_callback(SRef<Command_Receiver*> callback)
    {
        message_router_callback = callback;
    }

    SRef<Command_Receiver *> get_message_router_callback() { return message_router_callback;}

    virtual std::string get_mem_object_type() const { return "Media_Handler"; }

#ifdef DEBUG_OUTPUT
    virtual std::string get_debug_string();
#endif

    SRef<Media*> get_media(std::string sdpType);

private:
    void init();

    Mini_Sip* minisip;

    std::list< SRef<Media *> > media;

     std::list< SRef<Session *> > session_list;

    std::string ringtone_file;

    SRef<Realtime_Media_Stream_Sender *> sstream;

    SRef<Audio_Media *> audio_media;
    SRef<Ip_Provider *> ip_provider;
    SRef<Ip_Provider *> ip6_provider;
    SRef<Sip_Configuration *> config;

    SRef<Command_Receiver*> message_router_callback;

    void set_session_sound_settings( std::string callid, std::string side, bool turnOn );

    void session_call_recorder_start( std::string callid, bool start );

    SRef<Streams_Player *> streams_player;
};

#endif // MEDIA_HANDLER_H
