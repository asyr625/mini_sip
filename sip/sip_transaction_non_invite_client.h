#ifndef SIP_TRANSACTION_NON_INVITE_CLIENT_H
#define SIP_TRANSACTION_NON_INVITE_CLIENT_H

#include "sip_transaction.h"

class Sip_Request;

class Sip_Transaction_Non_Invite_Client : public Sip_Transaction_Client
{
public:
    Sip_Transaction_Non_Invite_Client(SRef<Sip_Stack_Internal*> stack_internal,
            int cseq,
            const std::string &cseq_method,
            const std::string &callid);

    virtual ~Sip_Transaction_Non_Invite_Client();

    virtual std::string get_mem_object_type() const { return "SipTransactionNonInvCli"; }
    virtual std::string get_name() { return "transaction_noninviteclient[branch="+get_branch() +
                ",type="+get_debug_trans_type()+"]"; }

    void set_up_state_machine();
private:
    bool a0_start_trying_request(const Sip_SMCommand &command);
    bool a1_trying_proceeding_1xx( const Sip_SMCommand &command);
    bool a2_trying_terminated_TimerFOrErr( const Sip_SMCommand &command);
    bool a3_proceeding_completed_non1xxresp( const Sip_SMCommand &command);
    bool a4_proceeding_proceeding_timerE( const Sip_SMCommand &command);
    bool a5_proceeding_proceeding_1xx( const Sip_SMCommand &command);
    bool a6_proceeding_terminated_transperrOrTimerF( const Sip_SMCommand &command);
    bool a7_trying_completed_non1xxresp(const Sip_SMCommand &command);
    bool a8_trying_trying_timerE(const Sip_SMCommand &command);
    bool a9_completed_terminated_timerK(const Sip_SMCommand &command);

    bool a10_completed_completed_anyresp(const Sip_SMCommand &command);

    SRef<Sip_Request*> _last_request;

    int _timerE;
};

#endif // SIP_TRANSACTION_NON_INVITE_CLIENT_H
