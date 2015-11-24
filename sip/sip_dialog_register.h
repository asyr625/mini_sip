#ifndef SIP_DIALOG_REGISTER_H
#define SIP_DIALOG_REGISTER_H

#include "sip_dialog.h"
#include "sip_smcommand.h"

class Sip_Dialog_Register : public Sip_Dialog
{
public:
    Sip_Dialog_Register(SRef<Sip_Stack*> stack, SRef<Sip_Identity*> identity);
    virtual ~Sip_Dialog_Register();


    virtual bool handle_command(const Sip_SMCommand &command);

    virtual std::string get_name(){return "SipDialogRegister["+get_dialog_config()->_sip_identity->get_sip_uri().get_ip()+"]";}

    virtual std::string get_mem_object_type() const { return "SipDialogRegister"; }

    void update_fail_count();
    uint32_t get_fail_count();

    bool get_gui_feedback();
    void set_gui_feedback(bool fb);

    void send_register();
private:
    bool a0_start_trying_register( const Sip_SMCommand &command);
    bool a1_trying_registred_2xx( const Sip_SMCommand &command);
    bool a2_trying_trying_40x( const Sip_SMCommand &command);
    bool a3_trying_askpassword_40x( const Sip_SMCommand &command);
    bool a5_askpassword_trying_setpassword( const Sip_SMCommand &command);
    bool a6_askpassword_registred_2xx( const Sip_SMCommand &command);
    bool a9_askpassword_failed_cancel( const Sip_SMCommand &command);
    bool a10_trying_failed_transporterror( const Sip_SMCommand &command);
    bool a12_registred_trying_proxyregister( const Sip_SMCommand &command);
    bool a13_failed_terminated_notransactions( const Sip_SMCommand &command);
    bool a14_trying_trying_1xx( const Sip_SMCommand &command);
    bool a15_service_unavailable( const Sip_SMCommand &command);

    void set_up_state_machine();

    uint32_t _fail_count;
    bool _gui_feedback;

    std::string _my_domain;
};

#endif // SIP_DIALOG_REGISTER_H
