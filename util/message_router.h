#ifndef MESSAGE_ROUTER_H
#define MESSAGE_ROUTER_H
#include "sobject.h"
#include "exception.h"
#include "command_string.h"

class Subsystem_Not_Found_Exception : public Exception
{
public:
    Subsystem_Not_Found_Exception(const char* what);
};

class Command_Receiver : public virtual SObject
{
public:
    virtual void handle_command(std::string subsystem, const Command_String& command) = 0;
    virtual Command_String handle_command_resp(std::string subsystem, const Command_String&) = 0;
};


class Message_Router_Internal;

class Message_Router : public Command_Receiver
{
public:
    Message_Router();
    Message_Router(const Message_Router&rhm);
    ~Message_Router();

    bool has_subsystem(std::string name);

    bool add_subsystem(std::string subsystem, SRef<Command_Receiver*> ss); //const Command_String& cmd);

    void handle_command(std::string subsystem, const Command_String& cmd);

    Command_String handle_command_resp(std::string subsystem, const Command_String &cmd);

    void clear();

private:
    Message_Router_Internal*    _internal;
};

#endif // MESSAGE_ROUTER_H
