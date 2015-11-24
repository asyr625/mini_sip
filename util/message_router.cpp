#include <list>
#include <string>

#include "message_router.h"

Subsystem_Not_Found_Exception::Subsystem_Not_Found_Exception(const char* what) : Exception(what)
{
}

class Route
{
public:
    std::string _ssname;
    SRef<Command_Receiver*> _dest;
};

class Message_Router_Internal
{
public:
    std::list<Route> _subsystems;
};

Message_Router::Message_Router()
{
    _internal = new Message_Router_Internal;
}

Message_Router::Message_Router(const Message_Router& rhm)
{
    _internal = new Message_Router_Internal(*rhm._internal);
}

Message_Router::~Message_Router()
{
    delete _internal;
}

bool Message_Router::has_subsystem(std::string name)
{
    std::list<Route>::iterator i;
    for ( i = _internal->_subsystems.begin(); i != _internal->_subsystems.end(); i++)
    {
        if ((*i)._ssname == name)
        {
            return true;
        }
    }
    return false;
}

bool Message_Router::add_subsystem(std::string subsystem, SRef<Command_Receiver*> ss)
{
    if( has_subsystem(subsystem) )
        return false;
    else
    {
        Route r;
        r._ssname = subsystem;
        r._dest = ss;
        _internal->_subsystems.push_back(r);
        return true;
    }
}


void Message_Router::handle_command(std::string subsystem, const Command_String& cmd)
{
#ifdef DEBUG_OUTPUT
    my_dbg("messagerouter") << "Message_Router:  To:"<<subsystem<<" Command:"<<cmd.get_string()<<endl;
#endif
    std::list<Route>::iterator i;
    for ( i = _internal->_subsystems.begin(); i != _internal->_subsystems.end(); ++i )
    {
        if ((*i)._ssname == subsystem)
        {
            (*i)._dest->handle_command(subsystem, cmd);
            return;
        }
    }
    throw Subsystem_Not_Found_Exception(subsystem.c_str());
}

Command_String Message_Router::handle_command_resp(std::string subsystem, const Command_String &cmd)
{
#ifdef DEBUG_OUTPUT
    my_dbg("messagerouter") << "Message_Router:  To(request):"<<subsystem<<" Command:"<< cmd.get_string()<< endl;
#endif
    std::list<Route>::iterator i;
    for ( i = _internal->_subsystems.begin(); i != _internal->_subsystems.end(); ++i )
    {
        if ((*i)._ssname == subsystem)
        {
            Command_String ret =(*i)._dest->handle_command_resp(subsystem, cmd);
#ifdef DEBUG_OUTPUT
            my_dbg("messagerouter") << "Message_Router:  Response from:"<<subsystem<<" Command:"<<ret.get_string()<<endl;
#endif
            return ret;
        }
    }
    throw Subsystem_Not_Found_Exception(subsystem.c_str());

    Command_String dummy("","");
    return dummy;
}

void Message_Router::clear()
{
    _internal->_subsystems.clear();
}
