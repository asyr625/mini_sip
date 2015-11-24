#ifndef SIP_TRANSACTION_NON_INVITE_SERVER_H
#define SIP_TRANSACTION_NON_INVITE_SERVER_H

#include "sip_transaction.h"

class Sip_Response;

class Sip_Transaction_Non_Invite_Server : public Sip_Transaction_Server
{
public:
    Sip_Transaction_Non_Invite_Server(SRef<Sip_Stack_Internal*> stack_internal,
            int cseq,
            const std::string &cseq_method,
            const std::string &branch,
            const std::string &callid);

    virtual ~Sip_Transaction_Non_Invite_Server();

    virtual std::string get_mem_object_type() const {return "SipTransactionNonInvServer";}
    virtual std::string get_name() { return "transaction_noninviteserver[branch="+get_branch() +
                ",type="+get_debug_trans_type()+"]"; }

    void set_up_state_machine();
private:
    bool a0_start_trying_request(const Sip_SMCommand &command);
    bool a1_trying_proceeding_1xx(const Sip_SMCommand &command);
    bool a2_trying_completed_non1xxresp(const Sip_SMCommand &command);
    bool a3_proceeding_completed_non1xxresp(const Sip_SMCommand &command);
    bool a4_proceeding_proceeding_request(const Sip_SMCommand &command);
    bool a5_proceeding_proceeding_1xx(const Sip_SMCommand &command);
    bool a6_proceeding_terminated_transperr(const Sip_SMCommand &command);
    bool a7_completed_completed_request(const Sip_SMCommand &command);
    bool a8_completed_terminated_transperr(const Sip_SMCommand &command);
    bool a9_completed_terminated_timerJ(const Sip_SMCommand &command);
    SRef<Sip_Response*> _last_response;
    int _timerJ;
};

#endif // SIP_TRANSACTION_NON_INVITE_SERVER_H
