#ifndef DEFAULT_DIALOG_HANDLER_H
#define DEFAULT_DIALOG_HANDLER_H

#include "sip_stack.h"
#include "sip_dialog.h"
#include "subsystem_media.h"

class Default_Dialog_Handler : public Sip_Default_Handler
{
public:
    Default_Dialog_Handler(SRef<Sip_Stack*> stack,
            SRef<Sip_Configuration*> pconf,
            SRef<Subsystem_Media*> subsystemMedia);

    virtual ~Default_Dialog_Handler();

    virtual std::string get_mem_object_type() const {return "DefaultDialogHandler";}

    virtual std::string get_name();

    virtual bool handle_command(const Sip_SMCommand &command);

    void handle_command(std::string subsystem, const Command_String &cmd);

    Command_String handle_command_resp(std::string subsystem, const Command_String &cmd);

private:
    bool handle_command_packet(SRef<Sip_Message*> pkt );
    bool handle_command_string(Command_String &cmdstr );

    void send_im_ok(SRef<Sip_Request*> immessage);

    void send_im(std::string msg, int im_seq_no, std::string toUri);

    SRef<Sip_Identity *> lookup_target(const Sip_Uri &uri);

    SRef<Sip_Stack*> _sip_stack;

    int _outside_dialog_seq_no;

    SRef<Sip_Configuration*> _conf; //phoneconf
    SRef<Subsystem_Media*> _subsystem_media;
};

#endif // DEFAULT_DIALOG_HANDLER_H
