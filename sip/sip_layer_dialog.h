#ifndef SIP_LAYER_DIALOG_H
#define SIP_LAYER_DIALOG_H

#include "sip_smcommand.h"
#include "mutex.h"

class Sip_Command_Dispatcher;
class Sip_Dialog;
class Sip_Default_Handler;

class Sip_Layer_Dialog : public Sip_SMCommand_Receiver
{
public:
    Sip_Layer_Dialog(SRef<Sip_Command_Dispatcher*> dispatcher);

    ~Sip_Layer_Dialog();

    void set_default_dialog_command_handler(SRef<Sip_Default_Handler*> cb);

    SRef<Sip_Default_Handler*> get_default_dialog_command_handler();

    void add_dialog(SRef<Sip_Dialog*> d);

    SRef<Sip_Dialog*> get_dialog(std::string call_id);

    bool remove_dialog(std::string call_id);

    virtual std::string get_mem_object_type() const { return "SipLayerDialog"; }

    virtual bool handle_command(const Sip_SMCommand &c);

    std::list<SRef<Sip_Dialog*> > get_dialogs();

private:
    SRef<Sip_Default_Handler*> _default_handler;

    std::map<std::string, SRef<Sip_Dialog*> > _dialogs;

    SRef<Sip_Command_Dispatcher*> _dispatcher;

    Mutex _dialog_list_lock;
};

#endif // SIP_LAYER_DIALOG_H
