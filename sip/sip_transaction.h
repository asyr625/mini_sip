#ifndef SIP_TRANSACTION_H
#define SIP_TRANSACTION_H

#include "state_machine.h"
#include "sip_smcommand.h"
#include "sip_dialog_config.h"

#include "sip_layer_transaction.h"

class Sip_Dialog;
class Sip_Message;
class Sip_Stack_Internal;
class Sip_Command_Dispatcher;
class Sip_Layer_Transport;

class Sip_Transaction : public State_Machine<Sip_SMCommand, std::string>
{
public:
    Sip_Transaction(SRef<Sip_Stack_Internal*> stack_internal,
            int cseq,
            const std::string &cseq_method,
            const std::string &branch,
            const std::string &callid,
            bool client);

    virtual ~Sip_Transaction();

    static SRef<Sip_Transaction*> create(SRef<Sip_Stack_Internal*> stack_internal,
            SRef<Sip_Request*> req,
            bool from_TU,
            bool handle_ack=false);

    virtual std::string get_name()=0;

    virtual bool handle_command(const Sip_SMCommand &command);

    virtual void handle_timeout(const std::string &c);

    std::string get_branch();
    void set_branch(std::string branch);

    std::string get_transaction_id(){ return get_branch() + get_cseq_method()+ (_is_client ? "c" : "s"); }

    void send(SRef<Sip_Message*>  pack, bool add_via, std::string branch=""); // if not specified branch, use the attribute one - ok in most cases.
    void set_socket(Socket * sock);
    SRef<Socket *> get_socket();

    virtual std::string get_mem_object_type() const;
    void set_debug_trans_type(std::string t);
    std::string get_debug_trans_type();

    int get_cseq_no();
    std::string get_cseq_method();

    std::string get_call_id();

    bool a1000_anyState_terminated_canceltransaction(const Sip_SMCommand &command);

    bool is_unreliable();

protected:
    void set_cseq_no(int n) { _cseq_no = n; }
    SRef<Sip_Command_Dispatcher*> _dispatcher;
    SRef<Sip_Stack_Internal*> _sip_stack_internal;
    SRef<Sip_Layer_Transport*> _transport_layer;
    SRef<Socket *> _socket;

    std::string _call_id;

private:
    int _cseq_no;
    std::string _cseq_method;
    std::string _branch;
    bool _is_client;

    std::string _debug_trans_type;
};

class Sip_Transaction_Client : public Sip_Transaction
{
public:
    Sip_Transaction_Client(SRef<Sip_Stack_Internal*> stack_internal,
            int cseq,
            const std::string &cseq_method,
            const std::string &branch,
            const std::string &callid);
    ~Sip_Transaction_Client();
};

class Sip_Transaction_Server : public Sip_Transaction
{
public:
    Sip_Transaction_Server(SRef<Sip_Stack_Internal*> stack_internal,
            int cseq,
            const std::string &cseq_method,
            const std::string &branch,
            const std::string &callid);
    ~Sip_Transaction_Server();
};

#endif // SIP_TRANSACTION_H
