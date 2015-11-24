#include "sip_stack.h"
#include "sip_command_dispatcher.h"

#define STACK (*(SRef<Sip_Stack_Internal*> *) _sip_stack_internal)

Sip_Stack::Sip_Stack(SRef<Sip_Stack_Config *> stack_config)
{
    Sip_Stack_Internal *istack = new Sip_Stack_Internal(stack_config);

    _sip_stack_internal = new SRef<Sip_Stack_Internal*>(istack);
}


Sip_Stack::~Sip_Stack()
{
    free();
}

void Sip_Stack::free()
{
    if ( _sip_stack_internal )
    {
        SRef<Sip_Stack_Internal*> * mref_ptr = (SRef<Sip_Stack_Internal*> *) _sip_stack_internal;
        (*mref_ptr)->free();

        delete mref_ptr;
        _sip_stack_internal=NULL;
    }
}

void Sip_Stack::set_transaction_handles_ack(bool trans_handle_ack)
{
    STACK->set_transaction_handles_ack(trans_handle_ack);
}

void Sip_Stack::set_default_dialog_command_handler(SRef<Sip_Default_Handler*> cb)
{
    STACK->set_default_dialog_command_handler(cb);
}

void Sip_Stack::run()
{
    STACK->run();
}

void Sip_Stack::stop_running()
{
    STACK->stop_running();
}

void Sip_Stack::handle_command(std::string subsystem, const Command_String &cmd)
{
    STACK->get_default_dialog_command_handler()->handle_command(subsystem, cmd);
}

Command_String Sip_Stack::handle_command_resp(std::string subsystem, const Command_String &cmd)
{
    return STACK->get_default_dialog_command_handler()->handle_command_resp(subsystem, cmd);
}

bool Sip_Stack::handle_command(const Command_String &cmd)
{
    return STACK->handle_command(cmd);
}

bool Sip_Stack::handle_command(const Sip_SMCommand &cmd)
{
    return STACK->handle_command(cmd);
}

void Sip_Stack::enqueue_timeout(SRef<Sip_Dialog*> receiver, const Sip_SMCommand &cmd)
{
    STACK->get_dispatcher()->enqueue_timeout(receiver, cmd);
}

void Sip_Stack::enqueue_command(const Sip_SMCommand &cmd, int queue)
{
    STACK->get_dispatcher()->enqueue_command(cmd, queue);
}

std::string Sip_Stack::create_client_transaction(SRef<Sip_Request*> req)
{
    return STACK->create_client_transaction(req);
}

void Sip_Stack::set_callback(SRef<Command_Receiver*> callback)
{
    STACK->set_callback(callback);
}

SRef<Command_Receiver*> Sip_Stack::get_callback()
{
    return STACK->get_callback();
}

void Sip_Stack::set_conf_callback(SRef<Command_Receiver*> callback)
{
    STACK->set_conf_callback(callback);
}

SRef<Command_Receiver *> Sip_Stack::get_conf_callback()
{
    return STACK->get_conf_callback();
}

void Sip_Stack::add_dialog(SRef<Sip_Dialog*> d)
{
    STACK->add_dialog(d);
}

SRef<Sip_Timers*> Sip_Stack::get_timers()
{
    return STACK->get_timers();
}

SRef<Sip_Stack_Config*> Sip_Stack::get_stack_config()
{
    return STACK->get_stack_config();
}

void Sip_Stack::add_supported_extension(std::string extension)
{
    return STACK->add_supported_extension(extension);
}

std::string Sip_Stack::get_all_supported_extensions_str()
{
    return STACK->get_all_supported_extensions_str();
}

bool Sip_Stack::supports(std::string extension)
{
    return STACK->supports(extension);
}

SRef<Timeout_Provider<std::string, SRef<State_Machine<Sip_SMCommand,std::string>*> > *> Sip_Stack::get_timeout_provider()
{
    return STACK->get_timeout_provider();
}

void Sip_Stack::set_dialog_management(SRef<Sip_Dialog*> mgmt)
{
    STACK->get_dispatcher()->set_dialog_management(mgmt);
}

std::list<SRef<Sip_Dialog *> > Sip_Stack::get_dialogs()
{
    return STACK->get_dispatcher()->get_dialogs();
}

SRef<Sip_Dialog *> Sip_Stack::get_dialog (std::string call_id)
{
    return STACK->get_dispatcher()->get_dialog(call_id);
}

void Sip_Stack::start_servers()
{
    STACK->start_servers();
}

void Sip_Stack::start_udp_server()
{
    STACK->start_server( "UDP" );
}
void Sip_Stack::start_tcp_server()
{
    STACK->start_server( "TCP" );
}

void Sip_Stack::start_tls_server()
{
    STACK->start_server( "TLS" );
}

void Sip_Stack::start_server( const std::string &transport_name )
{
    STACK->start_server( transport_name );
}

int Sip_Stack::get_local_sip_port(bool uses_stun, const std::string &transport)
{
    return STACK->get_local_sip_port(uses_stun, transport);
}


void Sip_Stack::set_debug_print_packets(bool enable)
{
    ::set_debug_print_packets(enable);
}

bool Sip_Stack::get_debug_print_packets()
{
    return ::get_debug_print_packets();
}

std::string Sip_Stack::get_stack_status_debug_string()
{
    return STACK->get_stack_status_debug_string();
}

void Sip_Stack::set_inform_transaction_terminate(bool doInform)
{
    return STACK->set_inform_transaction_terminate(doInform);
}


