#ifndef SIP_TRANSACTION_INVITE_CLIENT_H
#define SIP_TRANSACTION_INVITE_CLIENT_H

#include "sip_transaction.h"

class Sip_Response;

class Sip_Transaction_Invite_Client : public Sip_Transaction_Client
{
public:
    Sip_Transaction_Invite_Client(SRef<Sip_Stack_Internal*> stack_internal,
            int cseq,
            const std::string &cseq_method,
            const std::string &callid);

    virtual ~Sip_Transaction_Invite_Client();

    virtual std::string get_mem_object_type() const {return "SipTransactionInvCli";}
    virtual std::string get_name(){ return "transaction_invite_client[branch="+ get_branch() + "]"; }

    void set_up_state_machine();

    void set_dialog_route_set(SRef<Sip_Response *> resp);

    void send_ack(SRef<Sip_Response *> resp, bool provisional=false);

private:
    bool a0_start_calling_INVITE( const Sip_SMCommand &command);
    bool a1_calling_calling_timerA( const Sip_SMCommand &command);

    bool a2_calling_proceeding_1xx( const Sip_SMCommand &command);
    bool a3_calling_completed_resp36( const Sip_SMCommand &command);
    bool a4_calling_terminated_ErrOrTimerB( const Sip_SMCommand &command);
    bool a5_calling_terminated_2xx( const Sip_SMCommand &command);
    bool a6_proceeding_proceeding_1xx( const Sip_SMCommand &command);
    bool a7_proceeding_terminated_2xx( const Sip_SMCommand &command);
    bool a8_proceeding_completed_resp36( const Sip_SMCommand &command);
    bool a9_completed_completed_resp36( const Sip_SMCommand &command);
    bool a10_completed_terminated_TErr( const Sip_SMCommand &command);
    bool a11_completed_terminated_timerD( const Sip_SMCommand &command);

    SRef<Sip_Request*> _last_invite;
    int _timerA;
};

#endif // SIP_TRANSACTION_INVITE_CLIENT_H
