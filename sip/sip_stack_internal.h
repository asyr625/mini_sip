#ifndef SIP_STACK_INTERNAL_H
#define SIP_STACK_INTERNAL_H

#include "thread.h"
#include "message_router.h"
#include "timeout_provider.h"
#include "state_machine.h"

#include "sip_timers.h"
#include "sip_smcommand.h"
//#include "sip_command_dispatcher.h"

class Sip_Command_Dispatcher;
class Sip_Dialog;
class Sip_Stack_Config;
class Sip_Transport;
class Sip_Default_Handler;
class Sip_Transport_Config;
class Sip_Request;

class Sip_Stack_Internal : public Sip_SMCommand_Receiver,  public Runnable
{
public:
    Sip_Stack_Internal( SRef<Sip_Stack_Config*> stack_config );

    void set_transaction_handles_ack(bool transHandleAck);

    void set_default_dialog_command_handler(SRef<Sip_Default_Handler*> cb);
    SRef<Sip_Default_Handler*> get_default_dialog_command_handler();

    virtual std::string get_mem_object_type() const {return "SipStackInternal";}

    virtual void run();
    virtual void stop_running();

    SRef<Sip_Command_Dispatcher*> get_dispatcher();

    bool handle_command(const Command_String &cmd);

    bool handle_command(const Sip_SMCommand &command);

    void set_callback(SRef<Command_Receiver*> callback);	//Rename to setMessageRouter_callback?
    SRef<Command_Receiver *> get_callback();

    void set_conf_callback(SRef<Command_Receiver*> callback); // Hack to make the conference calling work - should not be here FIXME
    SRef<Command_Receiver *> get_conf_callback();

    void add_dialog(SRef<Sip_Dialog*> d);

    SRef<Timeout_Provider<std::string, SRef<State_Machine<Sip_SMCommand,std::string>*> > *> get_timeout_provider();

    SRef<Sip_Timers*> get_timers();
    SRef<Sip_Stack_Config*> get_stack_config();

    void add_supported_extension(std::string extension);
    std::string get_all_supported_extensions_str();
    bool supports(std::string extension);

    std::string get_stack_status_debug_string();

    SRef<Sip_Transport_Config*> find_transport_config( const std::string &transport_name ) const;

    void start_servers();

    void stop_servers();

    void start_server( const std::string &transport_name );

    void stop_server( const std::string &transport_name );

    int get_local_sip_port(bool uses_stun, const std::string &transport);

    void free();

    std::string create_client_transaction(SRef<Sip_Request*> req);

    void set_inform_transaction_terminate(bool doinform);

protected:
    void start_sip_servers();
    void start_sips_servers();
    void start_servers(bool secure, int &pref_port );
    void start_server( SRef<Sip_Transport*> transport, int &port );

    void stop_servers( bool secure );
    void stop_server( SRef<Sip_Transport*> transport );

private:

    SRef<Sip_Timers*> _timers;
    SRef<Sip_Stack_Config *> _config;
    SRef<Command_Receiver*> _callback;

    SRef<Command_Receiver*> _conf_callback;	//hack to make conference calling work until the ConfMessageRouter is removed

    SRef<Sip_Command_Dispatcher*> _dispatcher;

    SRef<Timeout_Provider<std::string, SRef<State_Machine<Sip_SMCommand,std::string>*> > *> _timeout_provider;

    std::list<std::string> _sip_extensions;
};

void set_debug_print_packets(bool);
bool get_debug_print_packets();

#endif // SIP_STACK_INTERNAL_H
