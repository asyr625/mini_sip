#ifndef GUI_H
#define GUI_H
#include <iostream>

#include "sobject.h"
#include "command_string.h"
#include "thread.h"
#include "message_router.h"
#include "my_assert.h"

class Sip_Configuration;
class Contact_Db;

class Gui : public Runnable, public Command_Receiver
{
public:
    virtual ~Gui();

    virtual void set_sip_configuration(SRef<Sip_Configuration *> sipphoneconfig) = 0;

    virtual void set_contact_db(SRef<Contact_Db *> contactDb) = 0;

    virtual void handle_command(const Command_String &command) = 0;

    void handle_command(std::string subsystem, const Command_String &cmd)
    {
        my_assert(subsystem == "gui");
        handle_command(cmd);
    }

    Command_String handle_command_resp(std::string , const Command_String& )
    {
        std::cerr << "Warning: Gui::handle_command_resp called (BUG)"<<std::endl;
        Command_String ret("","");
        return ret;
    }

    virtual void set_callback(SRef<Command_Receiver*> cb);

    SRef<Command_Receiver*> get_callback();

    void send_command(std::string toSubsystem, const Command_String &cmd)
    {
        callback->handle_command(toSubsystem, cmd);
    }

    Command_String send_command_resp(std::string toSubsystem, const Command_String &cmd)
    {
        return callback->handle_command_resp(toSubsystem, cmd);
    }

    virtual bool config_dialog( SRef<Sip_Configuration *> conf ) = 0;

    virtual void run() = 0;
protected:
    SRef<Command_Receiver*> callback;
};

#endif // GUI_H
