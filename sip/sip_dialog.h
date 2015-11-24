#ifndef SIP_DIALOG_H
#define SIP_DIALOG_H

#include "state_machine.h"

#include "sip_smcommand.h"
#include "sip_response.h"
#include "sip_request.h"
#include "sip_dialog_config.h"
#include "sip_transaction.h"

#include "sip_stack.h"

class Sip_Authentication_Digest;

class Sip_Dialog_State
{
public:
    bool update_state( SRef<Sip_Request*> inv );
    bool update_state( SRef<Sip_Response*> resp);

    std::string _call_id;
    std::string _local_tag;
    std::string _remote_tag;

    int _seq_no;
    int _remote_seq_no;
    unsigned int _rseq_no;
    std::string _local_uri;

    std::string get_remote_target();
    std::string _remote_uri;
    std::string _remote_target;
    bool _secure;
    std::list<std::string> _route_set;

    bool _is_early;
    bool _is_established;
    bool _is_terminated;
    std::list<SRef<Sip_Authentication_Digest*> > _auths;
};

class Sip_Dialog : public Sip_SMCommand_Receiver, public State_Machine<Sip_SMCommand, std::string>
{
public:
    Sip_Dialog(SRef<Sip_Stack*> stack, SRef<Sip_Identity*> identity, std::string call_id);
    virtual ~Sip_Dialog();

    virtual void free();

    virtual std::string get_mem_object_type() const {return "Sip_Dialog";}

    virtual bool handle_command(const Sip_SMCommand &command);

    virtual std::string get_name() = 0;

    virtual void handle_timeout(const std::string &c);
    SRef<Sip_Dialog_Config*> get_dialog_config();

    std::string get_call_id();
    SRef<Sip_Stack*> get_sip_stack();

    void signal_if_no_transactions();
    std::list<std::string> get_route_set();
    void add_route( SRef<Sip_Request *> req );

    std::list<SRef<Sip_Transaction*> > get_transactions();
    Sip_Dialog_State _dialog_state;

    SRef<Sip_Request*> create_sip_message( const std::string &method );

    SRef<Sip_Request*> create_sip_message_ack( SRef<Sip_Request *> orig_req );

    SRef<Sip_Request*> create_sip_message_bye();
    SRef<Sip_Request*> create_sip_message_prack( SRef<Sip_Response*> resp );

    SRef<Sip_Request*> create_sip_message_refer( const std::string &referred_uri );

    SRef<Sip_Response*> create_sip_response( SRef<Sip_Request*> req, int status, const std::string &reason );

    void send_sip_message( SRef<Sip_Message*> msg, int queue = HIGH_PRIO_QUEUE );

    bool update_authentications( SRef<Sip_Response*> resp );
    void add_authorizations( SRef<Sip_Request*> req );

    const std::string &find_unauthenticated_realm() const;
    bool add_credential( SRef<Sip_Credential*> credential );

    std::string get_dialog_debug_string();
    std::list<std::string> get_required_unsupported( SRef<Sip_Message*> msg );

    bool reject_unsupported( SRef<Sip_Request*> req );
protected:

    bool update_authentication(SRef<Sip_Header_Value_Proxy_Authenticate*> auth);
    void clear_authentications();
private:
    SRef<Sip_Dialog_Config*> _call_config;

    SRef<Sip_Request*> create_sip_message_seq( const std::string &method, int seq_no );
};

#endif // SIP_DIALOG_H
