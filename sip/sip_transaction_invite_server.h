#ifndef SIP_TRANSACTION_INVITE_SERVER_H
#define SIP_TRANSACTION_INVITE_SERVER_H

#include "sip_transaction.h"

class Sip_Response;

class Sip_Transaction_Invite_Server : public Sip_Transaction_Server
{
public:
    Sip_Transaction_Invite_Server(SRef<Sip_Stack_Internal*> stack_internal,
            int cseq,
            const std::string &cseq_method,
            const std::string &branch,
            const std::string &callid);

    virtual ~Sip_Transaction_Invite_Server();

    virtual std::string get_mem_object_type() const {return "SipTransactionInvServer";}
    virtual std::string get_name(){return "transaction_INVITE_responder[branch="+get_branch()+"]";}

    void set_up_state_machine();

    void set_dialog_route_set(SRef<Sip_Request*> inv);


    void send_trying();
    void send_ok();
    void send_ringing();
    void send_reject();

protected:
    SRef<Sip_Response*> _last_response;
    SRef<Sip_Response*> _last_reliable_response;
    int _timerG;
    int _timer_rel1xx_resend;

private:
    bool a0_start_proceeding_INVITE( const Sip_SMCommand &command);
    bool a1_proceeding_proceeding_INVITE( const Sip_SMCommand &command);
    bool a2_proceeding_proceeding_1xx( const Sip_SMCommand &command);
    bool a3_proceeding_completed_resp36( const Sip_SMCommand &command);
    bool a4_proceeding_terminated_err( const Sip_SMCommand &command);
    bool a5_proceeding_terminated_2xx( const Sip_SMCommand &command);
    bool a6_completed_completed_INVITE( const Sip_SMCommand &command);
    bool a7_completed_confirmed_ACK( const Sip_SMCommand &command);
    bool a8_completed_completed_timerG( const Sip_SMCommand &command);
    bool a9_completed_terminated_errOrTimerH( const Sip_SMCommand &command);
    bool a10_confirmed_terminated_timerI( const Sip_SMCommand &command);
    bool a11_confirmed_confirmed_ACK(const Sip_SMCommand&command);
    bool a20_proceeding_proceeding_timerRel1xxResend( const Sip_SMCommand &command);

    bool _user_has_accepted;
    bool _user_has_rejected;

    std::string _key_mgmt;
    int32_t _key_mgmt_method;
};

#endif // SIP_TRANSACTION_INVITE_SERVER_H
