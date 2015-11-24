#ifndef SIP_TRANSACTION_INVITE_SERVER_UA_H
#define SIP_TRANSACTION_INVITE_SERVER_UA_H

#include "sip_transaction_invite_server.h"

class Sip_Transaction_Invite_Server_Ua : public Sip_Transaction_Invite_Server
{
public:
    Sip_Transaction_Invite_Server_Ua(SRef<Sip_Stack_Internal*> stack_internal,
            int cseq,
            const std::string &cseq_method,
            const std::string &branch,
            const std::string &callid);

    virtual ~Sip_Transaction_Invite_Server_Ua();

    virtual std::string get_mem_object_type() const {return "SipTransactionInvServer";}
    virtual std::string get_name(){return "transaction_INVITE_responder[branch="+get_branch()+"]";}

    void change_state_machine();

private:
    bool a1001_proceeding_completed_2xx( const Sip_SMCommand &command);
};

#endif // SIP_TRANSACTION_INVITE_SERVER_UA_H
