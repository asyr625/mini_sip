#include <iostream>
#include <algorithm>

#include "sip_stack_internal.h"
#include "exception.h"
#include "network_exception.h"
#include "sip_command_dispatcher.h"
#include "sip_command_string.h"
#include "sip_layer_dialog.h"
#include "sip_layer_transaction.h"
#include "sip_layer_transport.h"
#include "sip_transport.h"

Sip_Stack_Internal::Sip_Stack_Internal( SRef<Sip_Stack_Config*> stack_config )
{
    _timers = new Sip_Timers;
    this->_config = stack_config;
    _timeout_provider = new Timeout_Provider<std::string, SRef<State_Machine<Sip_SMCommand,std::string>*> >;
    //
}


SRef<Sip_Command_Dispatcher*> Sip_Stack_Internal::get_dispatcher()
{
    return _dispatcher;
}


void Sip_Stack_Internal::free()
{
    _timeout_provider->stop_thread();
    _timeout_provider = NULL;
    set_callback(NULL);
    set_conf_callback(NULL);

    _dispatcher->free();
    _dispatcher = NULL;
}

SRef<Sip_Stack_Config*> Sip_Stack_Internal::get_stack_config()
{
    return _config;
}

void Sip_Stack_Internal::set_default_dialog_command_handler(SRef<Sip_Default_Handler*> cb)
{
    _dispatcher->get_layer_dialog()->set_default_dialog_command_handler(cb);
}

SRef<Sip_Default_Handler*> Sip_Stack_Internal::get_default_dialog_command_handler()
{
    return _dispatcher->get_layer_dialog()->get_default_dialog_command_handler();
}

void Sip_Stack_Internal::set_transaction_handles_ack(bool transHandleAck)
{
    _dispatcher->get_layer_transaction()->do_handle_ack(transHandleAck);
}

void Sip_Stack_Internal::set_callback(SRef<Command_Receiver*> callback)
{
    this->_callback = callback;
    _dispatcher->set_callback(_callback);
}

SRef<Command_Receiver *> Sip_Stack_Internal::get_callback()
{
    return _callback;
}

void Sip_Stack_Internal::set_conf_callback(SRef<Command_Receiver*> callback)
{
    this->_conf_callback = callback;
}

SRef<Command_Receiver *> Sip_Stack_Internal::get_conf_callback()
{
    return _conf_callback;
}


void Sip_Stack_Internal::run()
{
#ifdef DEBUG_OUTPUT
    set_thread_name("SipStack");
#endif
    _dispatcher->run();
}

void Sip_Stack_Internal::stop_running()
{
    _dispatcher->stop_running();
}

bool Sip_Stack_Internal::handle_command(const Sip_SMCommand &command)
{
    _dispatcher->enqueue_command(command, LOW_PRIO_QUEUE);
    return true;
}

bool Sip_Stack_Internal::handle_command(const Command_String &cmd)
{
    Sip_SMCommand c(cmd, Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer);
    return handle_command(c);
}


void Sip_Stack_Internal::add_dialog(SRef<Sip_Dialog*> d)
{
    _dispatcher->add_dialog(d);
}

SRef<Timeout_Provider<std::string, SRef<State_Machine<Sip_SMCommand,std::string>*> > *> Sip_Stack_Internal::get_timeout_provider()
{
    return _timeout_provider;
}

SRef<Sip_Timers*> Sip_Stack_Internal::get_timers()
{
    return _timers;
}


void Sip_Stack_Internal::add_supported_extension(std::string extension)
{
    _sip_extensions.push_back(extension);
}

bool Sip_Stack_Internal::supports(std::string extension)
{
    std::list<std::string>::iterator iter;
    for(iter = _sip_extensions.begin(); iter != _sip_extensions.end(); ++iter)
    {
        if( *iter == extension )
            return true;
    }
    return false;
}

std::string Sip_Stack_Internal::get_all_supported_extensions_str()
{
    bool first = true;
    std::string ret;
    std::list<std::string>::iterator iter;
    for(iter = _sip_extensions.begin(); iter != _sip_extensions.end(); ++iter)
    {
        if(!first)
        {
            ret = ret + ",";
        }
        else
        {
            first = false;
        }
        ret = ret + (*iter);
    }
    return ret;
}


std::string Sip_Stack_Internal::get_stack_status_debug_string()
{
    return "";
}

struct Transport_Config_Cmp
{
    typedef SRef<Sip_Transport_Config*> first_argument_type;
    typedef const std::string second_argument_type;
    typedef bool  result_type;

    result_type operator()( first_argument_type config,
                            second_argument_type name ) const
    {
        return config->get_name() == name;
    }
};

SRef<Sip_Transport_Config*> Sip_Stack_Internal::find_transport_config( const std::string &transport_name ) const
{
    std::list<SRef<Sip_Transport_Config*> >::const_iterator iter =
            std::find_if(_config->transports.begin(), _config->transports.end(), std::bind2nd(Transport_Config_Cmp(), transport_name) );
    if( iter == _config->transports.end() )
        return NULL;
    return *iter;
}

void Sip_Stack_Internal::start_servers()
{
    start_sip_servers();
    start_sips_servers();
}

void Sip_Stack_Internal::start_sip_servers()
{
    int port = _config->used_local_sip_port;
    try
    {
        start_servers(false,port);
    }
    catch(const Bind_Failed &bf)
    {
        stop_servers(false);
        port = 0;
        start_servers(false, port);
    }
}

void Sip_Stack_Internal::start_sips_servers()
{
    int port = _config->prefered_local_sips_port;
    try
    {
        start_servers(true,port);
    }
    catch(const Bind_Failed &bf)
    {
        stop_servers(true);
        port = 0;
        start_servers(true, port);
    }
}


void Sip_Stack_Internal::start_servers( bool secure, int &pref_port )
{
    std::list<SRef<Sip_Transport_Config*> >::const_iterator iter;
    std::list<SRef<Sip_Transport_Config*> >::const_iterator iter_lst = _config->transports.end();
    for( iter = _config->transports.begin(); iter != iter_lst; ++iter)
    {
        SRef<Sip_Transport_Config*> transport_config = (*iter);
        std::string name = transport_config->get_name();

        if( !transport_config->is_enabled() )
            continue;

        SRef<Sip_Transport*> transport = Sip_Transport_Registry::get_instance()->find_transport_by_name(name);
        if(!transport)
        {
            my_err << "Failed to start " << transport->get_name() << " server, unsupported" << std::endl;
            continue;
        }

        if(secure != transport->is_secure() )
        {
            continue;
        }
        my_dbg << "SipStack: Starting " << name << " transport worker thread" << std::endl;
        start_server(transport, pref_port);
    }
}

void Sip_Stack_Internal::start_server( const std::string &transport_name )
{
    SRef<Sip_Transport*> transport = Sip_Transport_Registry::get_instance()->find_transport_by_name(transport_name);

    if( !transport )
    {
        my_err << "Failed to start " << transport_name << " server, unsupported" << std::endl;
        return;
    }

    int port = 0;
    if( transport->is_secure() )
        port = _config->prefered_local_sips_port;
    else
        port = _config->used_local_sip_port;

    start_server(transport, port);
}

void Sip_Stack_Internal::start_server( SRef<Sip_Transport*> transport, int &port )
{
    int external_udp_port = 0;
    std::string ip = _config->local_ip_string;

    if( transport->get_name() == "UDP" )
    {
        if( _config->external_contact_ip.size() > 0 )
        {
            ip = _config->external_contact_ip;
            external_udp_port = _config->external_contact_udp_port;
        }
    }


    bool done = false;
    int ntries = 8;
    while( !done && ntries > 0 )
    {
        try
        {
            _dispatcher->get_layer_transport()->start_server(transport, ip, _config->local_ip6_string,
                                                             port, external_udp_port,
                                                             _config->cert, _config->cert_db);
            done = true;
        }
        catch(Exception&e)
        {
            port = _config->external_contact_udp_port = rand() %32000 + 32000;
        }
        catch(...)
        {
            throw;
        }
        --ntries;
    }
}


void Sip_Stack_Internal::stop_servers( bool secure )
{
    std::list<SRef<Sip_Transport_Config*> >::const_iterator iter;
    std::list<SRef<Sip_Transport_Config*> >::const_iterator iter_lst = _config->transports.end();
    for( iter = _config->transports.begin(); iter != iter_lst; ++iter)
    {
        SRef<Sip_Transport_Config*> transport_config = (*iter);
        std::string name = transport_config->get_name();

        if( !transport_config->is_enabled() )
            continue;

        SRef<Sip_Transport*> transport = Sip_Transport_Registry::get_instance()->find_transport_by_name(name);
        if(!transport)
        {
            my_err << "Failed to start " << transport->get_name() << " server, unsupported" << std::endl;
            continue;
        }

        if(secure != transport->is_secure() )
        {
            continue;
        }
        my_dbg << "SipStack: Stopping " << name << " transport worker thread" << std::endl;
        stop_server(transport);
    }
}

void Sip_Stack_Internal::stop_server( const std::string &transport_name )
{
    SRef<Sip_Transport*> transport = Sip_Transport_Registry::get_instance()->find_transport_by_name(transport_name);

    if( !transport )
    {
        my_err << "Failed to start " << transport_name << " server, unsupported" << std::endl;
        return;
    }
    stop_server(transport);
}

void Sip_Stack_Internal::stop_server( SRef<Sip_Transport*> transport )
{
    try
    {
        _dispatcher->get_layer_transport()->stop_server(transport);
    }
    catch(Network_Exception&e)
    {
        my_dbg << "Error: Failed to stop " << transport->get_name() << ": "<< e.what()<<std::endl;
    }
}

int Sip_Stack_Internal::get_local_sip_port(bool uses_stun, const std::string &transport)
{
    return _dispatcher->get_layer_transport()->get_local_sip_port(transport);
}

std::string Sip_Stack_Internal::create_client_transaction(SRef<Sip_Request*> req)
{
    return _dispatcher->get_layer_transaction()->create_client_transaction(req);
}

void Sip_Stack_Internal::set_inform_transaction_terminate(bool doinform)
{
    _dispatcher->set_inform_transaction_terminate(doinform);
}

void Sip_Stack_Internal::stop_servers()
{

}
