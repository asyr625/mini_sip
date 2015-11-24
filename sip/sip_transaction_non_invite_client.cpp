#include "sip_transaction_non_invite_client.h"
#include "sip_command_dispatcher.h"
#include "sip_command_string.h"
#include "sip_stack_internal.h"
#include "sip_transition_utils.h"


bool Sip_Transaction_Non_Invite_Client::a0_start_trying_request(const Sip_SMCommand &command)
{
    if (transition_match(Sip_Message::anyType, command, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer)){
#ifdef DEBUG_OUTPUT
        setDebugTransType(command.get_command_packet()->getType() );
#endif
        _last_request = dynamic_cast<Sip_Request*>(*command.get_command_packet());
        if( is_unreliable() )
        {
            _timerE = _sip_stack_internal->get_timers()->getE();
            request_timeout(_timerE, "timerE");
        }
        send(*_last_request,true);

        request_timeout(_sip_stack_internal->get_timers()->getF(), "timerF");
        return true;
    }
    return false;
}

bool Sip_Transaction_Non_Invite_Client::a1_trying_proceeding_1xx( const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer,
            "1**"))
    {
        cancel_timeout("timerE");
        cancel_timeout("timerF");
        Sip_SMCommand cmd(
                command.get_command_packet(),
                Sip_SMCommand::transaction_layer,
                Sip_SMCommand::dialog_layer );
        _dispatcher->enqueue_command(cmd, HIGH_PRIO_QUEUE);

        return true;
    }
    return false;
}

bool Sip_Transaction_Non_Invite_Client::a2_trying_terminated_TimerFOrErr( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::transport_error, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer)
            || transition_match(command, "timerF", Sip_SMCommand::transaction_layer, Sip_SMCommand::transaction_layer))
    {
        cancel_timeout("timerE");
        cancel_timeout("timerF");

        Sip_SMCommand cmd(
                Command_String( _call_id, Sip_Command_String::transport_error),
                Sip_SMCommand::transaction_layer,
                Sip_SMCommand::dialog_layer);
        _dispatcher->enqueue_command(cmd, HIGH_PRIO_QUEUE);

        Sip_SMCommand cmdterminated(
            Command_String( get_transaction_id(), Sip_Command_String::transaction_terminated),
            Sip_SMCommand::transaction_layer,
            Sip_SMCommand::dispatcher);
        _dispatcher->enqueue_command( cmdterminated, HIGH_PRIO_QUEUE);

        return true;
    }
    return false;
}

bool Sip_Transaction_Non_Invite_Client::a3_proceeding_completed_non1xxresp( const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer,
            "2**\n3**\n4**\n5**\n6**"))
    {
        SRef<Sip_Response*> pack((Sip_Response *)*command.get_command_packet());
        cancel_timeout("timerE"); //no more retx of the request
        if( is_unreliable() ) //response re-tx timer
            request_timeout(_sip_stack_internal->get_timers()->getT4(),"timerK");
        else
            request_timeout(0,"timerK");

        //forward to TU
        SRef<Sip_Message*> pref(*pack);
        Sip_SMCommand cmd(pref,
                Sip_SMCommand::transaction_layer,
                Sip_SMCommand::dialog_layer);
        _dispatcher->enqueue_command(cmd, HIGH_PRIO_QUEUE);

        return true;
    }
    return false;
}

bool Sip_Transaction_Non_Invite_Client::a4_proceeding_proceeding_timerE( const Sip_SMCommand &command)
{
    if (transition_match(command, "timerE", Sip_SMCommand::transaction_layer, Sip_SMCommand::transaction_layer))
    {
        _timerE *= 2;
        if( _timerE > _sip_stack_internal->get_timers()->getT2() )
            _timerE = _sip_stack_internal->get_timers()->getT2();
        request_timeout(_timerE,"timerE");
        my_assert(!_last_request.is_null());
        _timerE = _sip_stack_internal->get_timers()->getT2();
        request_timeout(_timerE,"timerE");
        send( *_last_request, false);

        return true;
    }
    return false;
}

bool Sip_Transaction_Non_Invite_Client::a5_proceeding_proceeding_1xx( const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer,
            "1**"))
    {
        SRef<Sip_Response*> pack((Sip_Response *)*command.get_command_packet());
        SRef<Sip_Message*> pref(*pack);
        Sip_SMCommand cmd( pref,
                Sip_SMCommand::transaction_layer,
                Sip_SMCommand::dialog_layer);
        cancel_timeout("timerE");

        _dispatcher->enqueue_command( cmd, HIGH_PRIO_QUEUE );
        return true;
    }
    return false;
}

bool Sip_Transaction_Non_Invite_Client::a6_proceeding_terminated_transperrOrTimerF( const Sip_SMCommand &command)
{
    if( transition_match(command, Sip_Command_String::transport_error, Sip_SMCommand::transport_layer,
                Sip_SMCommand::transaction_layer)
            || transition_match(command, "timerF", Sip_SMCommand::transaction_layer,
                Sip_SMCommand::transaction_layer))
    {

        cancel_timeout("timerE");
        cancel_timeout("timerF");

        Sip_SMCommand cmd(
                Command_String(_call_id,Sip_Command_String::transport_error),
                Sip_SMCommand::transaction_layer,
                Sip_SMCommand::dialog_layer);

        _dispatcher->enqueue_command( cmd, HIGH_PRIO_QUEUE );

        Sip_SMCommand cmdterminated(
                Command_String( get_transaction_id(), Sip_Command_String::transaction_terminated),
                Sip_SMCommand::transaction_layer,
                Sip_SMCommand::dispatcher);
        _dispatcher->enqueue_command( cmdterminated, HIGH_PRIO_QUEUE );
        return true;
    }
    return false;
}

bool Sip_Transaction_Non_Invite_Client::a7_trying_completed_non1xxresp(const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer,
            "2**\n3**\n4**\n5**\n6**"))
    {
        cancel_timeout("timerE");
        cancel_timeout("timerF");
        //send command to TU
        Sip_SMCommand cmd( command.get_command_packet(), Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer);
        _dispatcher->enqueue_command( cmd, HIGH_PRIO_QUEUE );

        if( is_unreliable() )
            request_timeout(_sip_stack_internal->get_timers()->getK(), "timerK");
        else
            request_timeout( 0, "timerK");
        return true;
    }
    return false;
}

bool Sip_Transaction_Non_Invite_Client::a8_trying_trying_timerE(const Sip_SMCommand &command)
{
    if (transition_match(command, "timerE", Sip_SMCommand::transaction_layer, Sip_SMCommand::transaction_layer))
    {
        //no need to check if is_unreliable() ... _timerE will never be started anyway
        _timerE *= 2;
        if( _timerE > _sip_stack_internal->get_timers()->getT2() )
            _timerE = _sip_stack_internal->get_timers()->getT2();
        request_timeout(_timerE,"timerE");

        my_assert( !_last_request.is_null());
        send( *_last_request, false);

        return true;
    }
    return false;
}

bool Sip_Transaction_Non_Invite_Client::a9_completed_terminated_timerK(const Sip_SMCommand &command)
{
    if (transition_match(command, "timerK", Sip_SMCommand::transaction_layer, Sip_SMCommand::transaction_layer))
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

bool Sip_Transaction_Non_Invite_Client::a10_completed_completed_anyresp(const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer,
            "1**\n2**\n3**\n4**\n5**\n6**"))
    {
        return true;
    }
    return false;
}

void Sip_Transaction_Non_Invite_Client::set_up_state_machine()
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



    new State_Transition<Sip_SMCommand,std::string>(this, "transition_start_trying_request",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Client::a0_start_trying_request,
            s_start, s_trying);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_trying_proceeding_1xx",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Client::a1_trying_proceeding_1xx,
            s_trying, s_proceeding);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_trying_terminated_TimerFOrErr",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Client::a2_trying_terminated_TimerFOrErr,
            s_trying, s_terminated);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_proceeding_completed_non1xxresp",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Client::a3_proceeding_completed_non1xxresp,
            s_proceeding, s_completed);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_proceeding_proceeding__timerE",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Client::a4_proceeding_proceeding_timerE,
            s_proceeding, s_proceeding);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_proceeding_proceeding_1xx",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Client::a5_proceeding_proceeding_1xx,
            s_proceeding, s_proceeding);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_proceeding_terminated_transperrOrTimerF",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Client::a6_proceeding_terminated_transperrOrTimerF,
            s_proceeding, s_terminated);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_trying_completed_non1xxresp",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Client::a7_trying_completed_non1xxresp,
            s_trying, s_completed);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_trying_trying__timerE",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Client::a8_trying_trying_timerE,
            s_trying, s_trying);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_completed_terminated_timerK",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Client::a9_completed_terminated_timerK,
            s_completed, s_terminated);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_completed_completed_anyresp",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Non_Invite_Client::a10_completed_completed_anyresp,
            s_completed, s_completed);


    set_current_state(s_start);
}

Sip_Transaction_Non_Invite_Client::Sip_Transaction_Non_Invite_Client(SRef<Sip_Stack_Internal*> stack_internal,
        int cseq,
        const std::string &cseq_method,
        const std::string &callid)
    : Sip_Transaction_Client(stack_internal, cseq, cseq_method, "", callid),
      _last_request(NULL)
{
    set_up_state_machine();
}

Sip_Transaction_Non_Invite_Client::~Sip_Transaction_Non_Invite_Client()
{
}
