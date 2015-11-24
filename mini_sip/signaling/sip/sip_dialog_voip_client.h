#ifndef SIP_DIALOG_VOIP_CLIENT_H
#define SIP_DIALOG_VOIP_CLIENT_H

#include "sip_stack.h"
#include "sip_dialog.h"
#include "sip_response.h"
#include "sip_dialog_voip.h"
#include "sip_configuration.h"

class Sip_Dialog_Voip_Client : public Sip_Dialog_Voip
{
public:
    Sip_Dialog_Voip_Client(SRef<Sip_Stack*> stack, SRef<Sip_Identity*> ident,
                           bool stun, bool anat, SRef<Session *> s, std::string cid = "");

    virtual ~Sip_Dialog_Voip_Client();

    virtual std::string get_mem_object_type() const { return "SipDialogVoipClient"; }

    virtual std::string get_name() { return "SipDialogVoipClient[callid="+ _dialog_state._call_id +"]"; }

private:
    bool use_anat;

    void set_up_state_machine();

    void send_invite();

    void send_ack();
    void send_prack(SRef<Sip_Response*>);

    bool a2001_start_calling_invite( const Sip_SMCommand &command);
    bool a2002_calling_calling_18X( const Sip_SMCommand &command);
    bool a2003_calling_calling_1xx( const Sip_SMCommand &command);
    bool a2004_calling_incall_2xx( const Sip_SMCommand &command);
    bool a2005_calling_termwait_CANCEL(const Sip_SMCommand &command);
    bool a2006_calling_termwait_cancel(const Sip_SMCommand &command);
    bool a2007_calling_termwait_36( const Sip_SMCommand &command);
    bool a2008_calling_calling_40X( const Sip_SMCommand &command);
    bool a2012_calling_termwait_2xx(const Sip_SMCommand &command);

    bool a2013_calling_termwait_transporterror( const Sip_SMCommand &command);
    bool a2017_any_any_2XX( const Sip_SMCommand &command);

    bool handle_rel1xx( SRef<Sip_Response*> resp );
};

#endif // SIP_DIALOG_VOIP_CLIENT_H
