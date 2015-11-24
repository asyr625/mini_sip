#include <iostream>

#include "sip_transaction.h"
#include "sip_command_dispatcher.h"
#include "sip_stack_internal.h"
#include "my_assert.h"
#include "string_utils.h"
#include "socket.h"

#include "sip_command_string.h"
#include "sip_transition_utils.h"
#include "sip_layer_transport.h"

#include "sip_transaction_invite_client.h"
#include "sip_transaction_non_invite_client.h"

#include "sip_transaction_invite_server_ua.h"
#include "sip_transaction_invite_server.h"
#include "sip_transaction_non_invite_server.h"

Sip_Transaction::Sip_Transaction(SRef<Sip_Stack_Internal*> stack_internal,
        int cseq,
        const std::string &cseq_method,
        const std::string &branch,
        const std::string &callid,
        bool client)
    :State_Machine<Sip_SMCommand, std::string>(stack_internal->get_timeout_provider() ),
      _sip_stack_internal(stack_internal),
      _cseq_no(cseq),
      _cseq_method(cseq_method),
      _branch(branch),
      _is_client(client)
{
    _dispatcher = stack_internal->get_dispatcher();
    my_assert(_dispatcher);
    _transport_layer = _dispatcher->get_layer_transport();
    my_assert(_transport_layer);

    _call_id = callid;
    if (branch == "")
    {
        _branch = "z9hG4bK" + itoa(rand());		//magic cookie plus random number
    }
}

Sip_Transaction::~Sip_Transaction()
{
}

SRef<Sip_Transaction*> Sip_Transaction::create(SRef<Sip_Stack_Internal*> stack_internal,
        SRef<Sip_Request*> req, bool from_TU, bool handle_ack)
{
    int seq_no = req->get_cseq();
    std::string seq_method = req->get_cseq_method();
    std::string call_id = req->get_call_id();
    std::string branch = req->get_branch();

#if DEBUG_OUTPUT
    my_dbg("signaling/sip") << "TRANSACTION_CREATE: "<< seq_method<<" "<<seq_no
                          <<" branch="<< branch <<" callid=" << call_id << " client="<< fromTU << std::endl;
#endif

    if( from_TU )
    {
        if( req->get_type() == "INVITE" )
            return new Sip_Transaction_Invite_Client(stack_internal, seq_no, seq_method, call_id);
        else
        {
            SRef<Sip_Transaction*> res = new Sip_Transaction_Non_Invite_Client(stack_internal, seq_no, seq_method, call_id);
            if( req->get_type() == "CANCEL" )
            {
                res->set_branch(branch);
            }
            return res;
        }
    }
    else
    {
        if( req->get_type() == "INVITE" )
        {
            if( handle_ack )
            {
                return new Sip_Transaction_Invite_Server_Ua(stack_internal, seq_no, seq_method, branch, call_id);
            }
            else
                return new Sip_Transaction_Invite_Server(stack_internal, seq_no, seq_method, branch, call_id);
        }
        else
            return new Sip_Transaction_Non_Invite_Server(stack_internal, seq_no, seq_method, branch, call_id);
    }
}

bool Sip_Transaction::a1000_anyState_terminated_canceltransaction(const Sip_SMCommand &command)
{
    if( transition_match(command, "cancel_transaction", Sip_SMCommand::dialog_layer , Sip_SMCommand::transaction_layer)
            && get_current_state_name() != "terminated")
    {
        Sip_SMCommand cmd_terminated(Command_String(get_transaction_id(), Sip_Command_String::transaction_terminated),
                                     Sip_SMCommand::transaction_layer, Sip_SMCommand::transaction_layer);

        _dispatcher->enqueue_command(cmd_terminated, HIGH_PRIO_QUEUE);
        return true;
    }
    else
        return false;
}


bool Sip_Transaction::handle_command(const Sip_SMCommand &command)
{
#ifdef DEBUG_OUTPUT
    my_dbg("signaling/sip") << "SipTransaction::handleCommand: tid<"<< get_transaction_id()<< "> got command "<< command <<std::endl;
#endif
    if( !(command.get_destination() == Sip_SMCommand::transaction_layer) )
    {
        std::cerr << "Transaction: returning false based on destination"<<std::endl;
        return false;
    }

    if( command.get_type() == Sip_SMCommand::COMMAND_PACKET &&
            command.get_command_packet()->get_cseq() != get_cseq_no() &&
            get_cseq_no() != -1)
    {
        std::cerr << "Transaction: returning false based on cseq"<<std::endl;
        return false;
    }

    if( command.get_type() == Sip_SMCommand::COMMAND_PACKET &&
            command.get_command_packet()->get_call_id() != _call_id )
    {
        std::cerr << "Transaction: returning false based on callid"<<std::endl;
        return false;
    }
    return State_Machine<Sip_SMCommand, std::string>::handle_command(command);
}

void Sip_Transaction::handle_timeout(const std::string &c)
{
    Sip_SMCommand cmd( Command_String(get_transaction_id(),c),
        Sip_SMCommand::transaction_layer,
        Sip_SMCommand::transaction_layer);
    _dispatcher->enqueue_timeout( this, cmd );
}

std::string Sip_Transaction::get_branch()
{
    return _branch;
}

void Sip_Transaction::set_branch(std::string branch)
{
    _branch = branch;
}


void Sip_Transaction::send(SRef<Sip_Message*>  pack, bool add_via, std::string branch) // if not specified branch, use the attribute one - ok in most cases.
{
    if( branch == "")
        branch = _branch;

    if( pack->get_type() == Sip_Response::type )
        pack->set_socket(get_socket());
    _transport_layer->send_message(pack, branch, add_via);

    if( pack->get_type() != Sip_Response::type && pack->get_socket() )
        set_socket( *pack->get_socket() );

#ifdef DEBUG_OUTPUT
        my_dbg("signaling/sip") << "SipTransaction::send: WARNING: Ignoring created socket"<<std::endl;
#endif
        return;
}

void Sip_Transaction::set_socket(Socket * sock)
{
    _socket = sock;
}

SRef<Socket *> Sip_Transaction::get_socket()
{
    return _socket;
}

std::string Sip_Transaction::get_mem_object_type() const
{
    return "SipTransaction";
}
void Sip_Transaction::set_debug_trans_type(std::string t)
{
    _debug_trans_type = t;
}

std::string Sip_Transaction::get_debug_trans_type()
{
    return _debug_trans_type;
}

int Sip_Transaction::get_cseq_no()
{
    return _cseq_no;
}

std::string Sip_Transaction::get_cseq_method()
{
    return _cseq_method;
}

std::string Sip_Transaction::get_call_id()
{
    return _call_id;
}

bool Sip_Transaction::is_unreliable()
{
    if( !_socket )
        my_dbg("signaling/sip") << "FIXME: SipTransaction::isUnrealiable: socket not initialized. Returning _unreliable_transport_ by default" << std::endl;
    return !_socket || _socket->get_type() == SSOCKET_TYPE_UDP;
}


Sip_Transaction_Client::Sip_Transaction_Client(SRef<Sip_Stack_Internal*> stack_internal,
        int seq_no,
        const std::string &cseqm,
        const std::string &branch,
        const std::string &callid)
    : Sip_Transaction(stack_internal, seq_no, cseqm, branch, callid, true)
{

}

Sip_Transaction_Client::~Sip_Transaction_Client()
{
}

Sip_Transaction_Server::Sip_Transaction_Server(SRef<Sip_Stack_Internal*> stack_internal,
        int seq_no,
        const std::string &cseqm,
        const std::string &branch,
        const std::string &callid)
    : Sip_Transaction(stack_internal, seq_no, cseqm, branch, callid, false)
{
}

Sip_Transaction_Server::~Sip_Transaction_Server()
{
}
