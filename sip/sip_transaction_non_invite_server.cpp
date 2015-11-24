#include <iostream>

#include "sip_transaction_non_invite_server.h"
#include "sip_transition_utils.h"
#include "sip_command_dispatcher.h"
#include "sip_command_string.h"
#include "sip_stack_internal.h"

bool Sip_Transaction_Non_Invite_Server::a0_start_trying_request(const Sip_SMCommand &command)
{
    if (transition_match( get_cseq_method(), command, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer))
    {
        SRef<Socket*> sock = command.get_command_packet()->get_socket();

        if( sock )
            set_socket( *sock );
        else
            set_socket( NULL );

        Sip_SMCommand cmd(command);
#ifdef DEBUG_OUTPUT
        /*server->*/set_debug_trans_type(command.get_command_packet()->get_type() );
#endif
        cmd.set_source(Sip_SMCommand::transaction_layer);
        cmd.set_destination(Sip_SMCommand::dialog_layer);

        _dispatcher->enqueue_command(cmd, HIGH_PRIO_QUEUE);

        return true;
    }
    return false;
}

bool Sip_Transaction_Non_Invite_Server::a1_trying_proceeding_1xx(const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer, "1**"))
    {
        _last_response = SRef<Sip_Response*>((Sip_Response*)*command.get_command_packet());
        send(command.get_command_packet(), false); //Do not add via header to responses
                            //they are copied from the request
        return true;
    }
    return false;
}

bool Sip_Transaction_Non_Invite_Server::a2_trying_completed_non1xxresp(const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer,
            "2**\n3**\n4**\n5**\n6**"))
    {

        _last_response = SRef<Sip_Response*>((Sip_Response*)*command.get_command_packet());
        send(command.get_command_packet(), false); 		//Do not add via header to responses
        request_timeout(/*64 * timerT1*/ _sip_stack_internal->get_timers()->getJ(), "timerJ");

        return true;
    }
    return false;
}

bool Sip_Transaction_Non_Invite_Server::a3_proceeding_completed_non1xxresp(const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer,
            "2**\n3**\n4**\n5**\n6**"))
    {
        _last_response = SRef<Sip_Response*>((Sip_Response*)*command.get_command_packet());
        send(command.get_command_packet(), false); 		//Do not add via header to responses
        if( is_unreliable() )
            request_timeout(_sip_stack_internal->get_timers()->getJ(), "timerJ");
        else
            request_timeout( 0, "timerJ");

        return true;
    }
    return false;
}

bool Sip_Transaction_Non_Invite_Server::a4_proceeding_proceeding_request(const Sip_SMCommand &command)
{
    my_err << "CESC: SipTransNIS::a4 ... " << std::endl;
    if (command.get_source()!=Sip_SMCommand::transport_layer)
        return false;

    if (command.get_type()!=Sip_SMCommand::COMMAND_PACKET)
    {
        return false;
    }
    if (command.get_type()==Sip_SMCommand::COMMAND_PACKET &&
            command.get_command_packet()->get_type()==Sip_Response::type){ //NOTICE: if Response, return
        return false;
    }

    my_assert( !_last_response.is_null());
    //We are re-sending last response, do not add via header
    send(SRef<Sip_Message*>(* _last_response),false);


    return true;
}

bool Sip_Transaction_Non_Invite_Server::a5_proceeding_proceeding_1xx(const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer, "1**"))
    {
        SRef<Sip_Response*> pack( (Sip_Response *)*command.get_command_packet());
        _last_response = pack;
        send(SRef<Sip_Message*>(*pack), false);

        return true;
    }
    return false;
}

bool Sip_Transaction_Non_Invite_Server::a6_proceeding_terminated_transperr(const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::transport_error, Sip_SMCommand::transport_layer,
                Sip_SMCommand::transaction_layer))
    {
        //inform TU
        Sip_SMCommand cmd(
                Command_String(_call_id,Sip_Command_String::transport_error),
                Sip_SMCommand::transaction_layer,
                Sip_SMCommand::dialog_layer);

        _dispatcher->enqueue_command( cmd, HIGH_PRIO_QUEUE );

        Sip_SMCommand cmdterminated(
            Command_String( get_transaction_id(), Sip_Command_String::transaction_terminated),
            Sip_SMCommand::transaction_layer,
            Sip_SMCommand::dispatcher);
        _dispatcher->enqueue_command( cmdterminated, HIGH_PRIO_QUEUE);

        return true;
    }
    return false;
}

bool Sip_Transaction_Non_Invite_Server::a7_completed_completed_request(const Sip_SMCommand &command)
{
    if (command.get_source()!=Sip_SMCommand::transport_layer)
        return false;

    if (command.get_type()!=Sip_SMCommand::COMMAND_PACKET){
        return false;
    }
    if (command.get_type()==Sip_SMCommand::COMMAND_PACKET &&
            command.get_command_packet()->get_type()==Sip_Response::type){
        return false;
    }
    my_assert( !_last_response.is_null());
    send(SRef<Sip_Message*>(* _last_response), false);		//We are re-sending response, do not add via header

    return true;
}

bool Sip_Transaction_Non_Invite_Server::a8_completed_terminated_transperr(const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::transport_error, Sip_SMCommand::transport_layer,
                Sip_SMCommand::transaction_layer))
    {

        cancel_timeout("timerJ");

        Sip_SMCommand cmd(
                Command_String(_call_id,Sip_Command_String::transport_error),
                Sip_SMCommand::transaction_layer,
                Sip_SMCommand::dialog_layer);

        _dispatcher->enqueue_command( cmd, HIGH_PRIO_QUEUE);

        Sip_SMCommand cmdterminated(
            Command_String( get_transaction_id(), Sip_Command_String::transaction_terminated),
            Sip_SMCommand::transaction_layer,
            Sip_SMCommand::dispatcher);
        _dispatcher->enqueue_command( cmdterminated, HIGH_PRIO_QUEUE );

        return true;
    }
    return false;
}

bool Sip_Transaction_Non_Invite_Server::a9_completed_terminated_timerJ(const Sip_SMCommand &command)
{
    if (transition_match(command, "timerJ", Sip_SMCommand::transaction_layer, Sip_SMCommand::transaction_layer))
    {
        Sip_SMCommand cmd(
            Command_String( get_transaction_id(), Sip_Command_String::transaction_terminated),
            Sip_SMCommand::transaction_layer,
            Sip_SMCommand::dispatcher);
        _dispatcher->enqueue_command( cmd, HIGH_PRIO_QUEUE );

        return true;
    }
    return false;
}

void Sip_Transaction_Non_Invite_Server::set_up_state_machine()
{
    State<Sip_SMCommand,std::string> *s_start=new State<Sip_SMCommand,std::string>(this,"start");
    add_state(s_start);

    State<Sip_SMCommand,std::string> *s_trying=new State<Sip_SMCommand,std::string>(this,"trying");
    add_state(s_trying);

    State<Sip_SMCommand,std::string> *s_proceeding=new State<Sip_SMCommand,std::string>(this,"proceeding");
    add_state(s_proceeding);

    State<Sip_SMCommand,std::string> *s_completed=new State<Sip_SMCommand,std::string>(this,"completed");
    add_state(s_completed);

    State<Sip_SMCommand,std::string> *s_terminated=new State<Sip_SMCommand,std::string>(this,"terminated");
    add_state(s_terminated);

    ///Set up transitions to enable cancellation of this transaction
    new State_Transition<Sip_SMCommand,std::string>(this, "transition_cancel_transaction",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction::a1000_anyState_terminated_canceltransaction,
            State_Machine<Sip_SMCommand,std::string>::any_state, s_terminated);

    //
    new State_Transition<Sip_SMCommand,std::string>(this, "transition_start_trying_request",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Server::a0_start_trying_request,
            s_start, s_trying);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_trying_proceeding_1xx",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Server::a1_trying_proceeding_1xx,
            s_trying, s_proceeding);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_trying_completed_non1xxresp",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Server::a2_trying_completed_non1xxresp,
            s_trying, s_completed);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_proceeding_completed_non1xxresp",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Server::a3_proceeding_completed_non1xxresp,
            s_proceeding, s_completed);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_proceeding_proceeding_request",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Server::a4_proceeding_proceeding_request,
            s_proceeding, s_proceeding);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_proceeding_proceeding_1xx",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Server::a5_proceeding_proceeding_1xx,
            s_proceeding, s_proceeding);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_proceeding_terminated_transperr",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Server::a6_proceeding_terminated_transperr,
            s_proceeding, s_terminated);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_completed_completed_request",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Server::a7_completed_completed_request,
            s_completed, s_completed);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_completed_terminated_transperr",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Server::a8_completed_terminated_transperr,
            s_completed, s_terminated);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_completed_terminated_timerJ",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Server::a9_completed_terminated_timerJ,
            s_completed, s_terminated);

    set_current_state(s_start);
}

Sip_Transaction_Non_Invite_Server::Sip_Transaction_Non_Invite_Server(SRef<Sip_Stack_Internal*> stack_internal,
        int cseq,
        const std::string &cseq_method,
        const std::string &branch,
        const std::string &callid)
    : Sip_Transaction_Server(stack_internal, cseq, cseq_method, branch, callid),
      _last_response(NULL)
{
    set_up_state_machine();
}

Sip_Transaction_Non_Invite_Server::~Sip_Transaction_Non_Invite_Server()
{

}
