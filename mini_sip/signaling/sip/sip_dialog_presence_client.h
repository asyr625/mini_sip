#ifndef SIP_DIALOG_PRESENCE_CLIENT_H
#define SIP_DIALOG_PRESENCE_CLIENT_H

#include "sip_dialog.h"

class Sip_Dialog_Presence_Client : public Sip_Dialog
{
public:
    Sip_Dialog_Presence_Client(SRef<Sip_Stack*> stack, SRef<Sip_Identity*> ident, bool use_stun);

    virtual ~Sip_Dialog_Presence_Client();

    virtual std::string get_mem_object_type() const {return "SipDialogPresenceClient";}

    virtual  std::string get_name(){return "SipDialogPresenceClient[callid="+_dialog_state._call_id +"]";}

    virtual bool handle_command(const Sip_SMCommand &c);

    void set_up_state_machine();

private:

    void send_subscribe();
    void send_notify_ok(SRef<Sip_Request*> notify);
    void create_subscribe_client_transaction();

    bool a0_start_trying_presence(const Sip_SMCommand &command);
    bool a1_X_subscribing_200OK(const Sip_SMCommand &command);
    bool a2_trying_retrywait_transperror(const Sip_SMCommand &command);
    bool a4_X_trying_timerTO(const Sip_SMCommand &command);
    bool a5_subscribing_subscribing_NOTIFY(const Sip_SMCommand &command);
    bool a6_subscribing_termwait_stoppresence(const Sip_SMCommand &command);
    bool a7_termwait_terminated_notransactions(const Sip_SMCommand &command);
    bool a8_trying_trying_40X(const Sip_SMCommand &command);
    bool a9_trying_retry_wait_failure(const Sip_SMCommand &command);

    SRef<Sip_Identity *> _to_uri;
    bool _use_stun;
};

#endif // SIP_DIALOG_PRESENCE_CLIENT_H
