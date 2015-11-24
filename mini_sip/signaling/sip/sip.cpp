#include <iostream>
using namespace std;

#include "sip.h"
#include "sip_command_string.h"
#include "sip_configuration.h"

Sip::Sip(SRef<Sip_Configuration*> config, SRef<Subsystem_Media*> subsystem_media)
    :_config(config), _subsystem_media(subsystem_media)
{
}

Sip::~Sip()
{
}

bool Sip::start()
{
    _thread = NULL;
    _thread = new Thread(this, Thread::Normal_Priority);
    return !_thread.is_null();
}

void Sip::stop()
{
    Command_String cmd("", Sip_Command_String::sip_stack_shutdown);
    Sip_SMCommand sc(cmd, Sip_SMCommand::dialog_layer, Sip_SMCommand::dispatcher);
    get_sip_stack()->handle_command(sc);
}

void Sip::join()
{
    if( _thread.is_null() )
        return ;
    _thread->join();
}

void Sip::run()
{
    try
    {
        _sip_stack->start_servers();
    }
    catch(Exception& e)
    {
        cerr << "ERROR: Sip::run() Exception thrown when creating "
            "SIP transport servers." << endl;
        cerr << e.what() << endl;
    }
#ifdef DEBUG_OUTPUT
    my_out << BOLD << "init 9/9: Registering Identities to registrar server" << PLAIN << endl;
#endif

    std::list< SRef<Sip_Identity*> >::iterator iter;
    std::list< SRef<Sip_Identity*> >::iterator iter_end = _config->_identities.end();
    for(iter = _config->_identities.begin(); iter != iter_end; ++iter)
    {
        if((*iter)->_register_to_proxy)
        {
            cerr << "Registering user "<< (*iter)->get_sip_uri().get_string() << " to proxy "
                 << (*iter)->get_sip_registrar()->get_uri().get_ip()<< ", requesting domain "
                 << (*iter)->get_sip_uri().get_ip() << endl;

            Command_String reg("",Sip_Command_String::proxy_register);

            reg["proxy_domain"] = (*iter)->get_sip_uri().get_ip();
            reg["identityId"] = (*iter)->get_id();

            Sip_SMCommand sipcmd(reg, Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer);

            _sip_stack->handle_command(sipcmd);
        }
    }
}

void Sip::set_media_handler( SRef<Subsystem_Media*> subsystem_media)
{
    this->_subsystem_media = subsystem_media;
}
