#include "sip_transaction_invite_server.h"
#include "sip_command_dispatcher.h"
#include "sip_transition_utils.h"
#include "sip_command_string.h"
#include "sip_stack_internal.h"


bool Sip_Transaction_Invite_Server::a0_start_proceeding_INVITE( const Sip_SMCommand &command)
{
    if (transition_match("INVITE", command, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer))
    {
        SRef<Socket*> sock = command.get_command_packet()->get_socket();

        if( sock )
            set_socket( *sock );
        else
            set_socket( NULL );

        Sip_SMCommand cmd(command);
        cmd.set_source(Sip_SMCommand::transaction_layer);
        cmd.set_destination(Sip_SMCommand::dialog_layer);
//		cmd.set_source(Sip_SMCommand::transaction);
        _dispatcher->enqueue_command(cmd, HIGH_PRIO_QUEUE);

        //update dialogs route set ... needed to add route headers to the ACK we are going to send

        // TODO/XXX/FIXME: implement this in the TU instead!!! --EE
        //setDialogRouteSet( (SipRequest*)*command.get_command_packet() );

        return true;
    }
    return false;
}

bool Sip_Transaction_Invite_Server::a1_proceeding_proceeding_INVITE( const Sip_SMCommand &command)
{
    if (transition_match("INVITE", command, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer))
    {
        SRef<Sip_Response*> resp = _last_response;
        if (resp.is_null())
        {
#ifdef DEBUG_OUTPUT
            my_err << FG_ERROR << "Invite server transaction failed to deliver response before remote side retransmitted. Bug?"<< PLAIN << endl;
#endif
        }
        else
        {
            send(SRef<Sip_Message*>(*resp), false);
        }
        return true;
    }
    return false;
}

bool Sip_Transaction_Invite_Server::a2_proceeding_proceeding_1xx( const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer, "1**"))
    {
        SRef<Sip_Response*> resp = (Sip_Response*)*command.get_command_packet();
        _last_response = resp;
        //no need for via header, it is copied from the request msg
#if 0
        if (resp->requires("100rel"))
        {
            _last_reliable_response = resp;
            //The order we want (no race):
            //  1. Create PRACK server transaction
            //  2. Request timeout
            //  3. send 1xx message

            SRef<Sip_Transaction*> pracktrans = new Sip_Transaction_Non_Invite_Server(sipStack,
                    /*SRef<SipDialog*>(this)*/ dialog,
                    resp->getCSeq()+1, // The PRACK transaction MUST be the next in sequence
                    "PRACK",
                    /*bye->getLastViaBranch()*/ "",
                    dialog->dialogState.callId);

            dialog->registerTransaction(pracktrans);

            _timer_rel1xx_resend = sipStack->get_timers()->getT1();
            request_timeout(_timer_rel1xx_resend,"_timer_rel1xx_resend");
        }
#endif
        send(command.get_command_packet(), false);
        return true;
    }
    return false;
}

bool Sip_Transaction_Invite_Server::a3_proceeding_completed_resp36( const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer,
            "3**\n4**\n5**\n6**"))
    {
        cancel_timeout("_timer_rel1xx_resend");
        _last_response = SRef<Sip_Response*>((Sip_Response*)*command.get_command_packet());
        if( is_unreliable() )
        {
            _timerG = _sip_stack_internal->get_timers()->getG();
            request_timeout(_timerG, "timerG");
        }
        request_timeout(_sip_stack_internal->get_timers()->getH(),"timerH");

        //no need for via header, it is copied from the request msg
        send(command.get_command_packet(), false);
        return true;
    }
    return false;
}

bool Sip_Transaction_Invite_Server::a4_proceeding_terminated_err( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::transport_error, Sip_SMCommand::transport_layer,
                        Sip_SMCommand::transaction_layer))
    {
        cancel_timeout("_timer_rel1xx_resend");

        Sip_SMCommand cmd( Command_String(_call_id, Sip_Command_String::transport_error),
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

bool Sip_Transaction_Invite_Server::a5_proceeding_terminated_2xx( const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer,  "2**"))
    {

        cancel_timeout("_timer_rel1xx_resend");
        _last_response = SRef<Sip_Response*>((Sip_Response*)*command.get_command_packet());

        //no need for via header, it is copied from the request msg
        send(command.get_command_packet(), false);

        Sip_SMCommand cmd(
                Command_String( get_transaction_id(), Sip_Command_String::transaction_terminated),
                Sip_SMCommand::transaction_layer,
                Sip_SMCommand::dispatcher);
        _dispatcher->enqueue_command( cmd, HIGH_PRIO_QUEUE );
        return true;
    }
    return false;
}

bool Sip_Transaction_Invite_Server::a6_completed_completed_INVITE( const Sip_SMCommand &command)
{
    if (transition_match("INVITE", command, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer))
    {
        SRef<Sip_Response*> resp = _last_response;
        send(SRef<Sip_Message*>(*resp), false);
        return true;
    }
    return false;
}

bool Sip_Transaction_Invite_Server::a7_completed_confirmed_ACK( const Sip_SMCommand &command)
{
    if (transition_match("ACK", command, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer))
    {
        cancel_timeout("timerG");//response re-tx
        cancel_timeout("timerH"); //wait for ACK reception
        if( is_unreliable() )
            request_timeout(_sip_stack_internal->get_timers()->getI(), "timerI");
        else
            request_timeout( 0, "timerI");

        // sending the ack to the dialog layer also this ACK can contain an SDP answer in its body
        // we want the functionality of the ack with sdp answer when implementimng 3pcc and
        // when adding another participant into the call

        Sip_SMCommand cmd(command);
                cmd.set_source(Sip_SMCommand::transaction_layer);
                cmd.set_destination(Sip_SMCommand::dialog_layer);
                _dispatcher->enqueue_command(cmd, HIGH_PRIO_QUEUE);
        return true;
    }
    return false;
}

bool Sip_Transaction_Invite_Server::a8_completed_completed_timerG( const Sip_SMCommand &command)
{
    if (transition_match(command, "timerG", Sip_SMCommand::transaction_layer, Sip_SMCommand::transaction_layer))
    {
        SRef<Sip_Response*> resp = _last_response;
        _timerG *= 2;
        if( _timerG > _sip_stack_internal->get_timers()->getT2() )
            _timerG = _sip_stack_internal->get_timers()->getT2();
        request_timeout( _timerG, "timerG");
        send(SRef<Sip_Message*>(*resp), false);
        return true;
    }
    return false;
}

bool Sip_Transaction_Invite_Server::a9_completed_terminated_errOrTimerH( const Sip_SMCommand &command)
{
    if ( transition_match(command, "timerH", Sip_SMCommand::transaction_layer, Sip_SMCommand::transaction_layer)
            || transition_match(command, Sip_Command_String::transport_error, Sip_SMCommand::transport_layer,
                Sip_SMCommand::transaction_layer) )
    {
        cancel_timeout("timerG");
        cancel_timeout("timerH");

        Sip_SMCommand cmd( Command_String(_call_id, Sip_Command_String::transport_error),
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

bool Sip_Transaction_Invite_Server::a10_confirmed_terminated_timerI( const Sip_SMCommand &command)
{
    if (transition_match(command, "timerI", Sip_SMCommand::transaction_layer, Sip_SMCommand::transaction_layer))
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

bool Sip_Transaction_Invite_Server::a11_confirmed_confirmed_ACK(const Sip_SMCommand&command)
{
    if (transition_match("ACK", command, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer))
    {
        return true;
    }
    return false;
}

bool Sip_Transaction_Invite_Server::a20_proceeding_proceeding_timerRel1xxResend( const Sip_SMCommand &command)
{
    if (transition_match(command, "_timer_rel1xx_resend", Sip_SMCommand::transaction_layer, Sip_SMCommand::transaction_layer))
    {
        _timer_rel1xx_resend*=2;
        request_timeout(_timer_rel1xx_resend, "_timer_rel1xx_resend");
        send(*_last_response, false); // second parameter is "bool addVia"
        return true;
    }
    return false;
}

void Sip_Transaction_Invite_Server::set_up_state_machine()
{
    State<Sip_SMCommand,std::string> *s_start=new State<Sip_SMCommand,std::string>(this,"start");
    add_state(s_start);

    State<Sip_SMCommand,std::string> *s_proceeding=new State<Sip_SMCommand,std::string>(this,"proceeding");
    add_state(s_proceeding);

    State<Sip_SMCommand,std::string> *s_completed=new State<Sip_SMCommand,std::string>(this,"completed");
    add_state(s_completed);

    State<Sip_SMCommand,std::string> *s_confirmed=new State<Sip_SMCommand,std::string>(this,"confirmed");
    add_state(s_confirmed);

    State<Sip_SMCommand,std::string> *s_terminated=new State<Sip_SMCommand,std::string>(this,"terminated");
    add_state(s_terminated);

    ///Set up transitions to enable cancellation of this transaction
    new State_Transition<Sip_SMCommand,std::string>(this, "transition_cancel_transaction",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Transaction::a1000_anyState_terminated_canceltransaction,
            State_Machine<Sip_SMCommand,std::string>::any_state, s_terminated);

    //
    new State_Transition<Sip_SMCommand,std::string>(this, "transition_start_proceeding_INVITE",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Server::a0_start_proceeding_INVITE,
            s_start, s_proceeding);
    new State_Transition<Sip_SMCommand,std::string>(this, "transition_proceeding_proceeding_INVITE",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Server::a1_proceeding_proceeding_INVITE,
            s_proceeding, s_proceeding);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_proceeding_proceeding_1xx",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Server::a2_proceeding_proceeding_1xx,
            s_proceeding, s_proceeding);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_proceeding_completed_resp36",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Server::a3_proceeding_completed_resp36,
            s_proceeding, s_completed);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_proceeding_terminated_Err",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Server::a4_proceeding_terminated_err,
            s_proceeding, s_terminated);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_proceeding_terminated_2xx",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Server::a5_proceeding_terminated_2xx,
            s_proceeding, s_terminated);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_completed_completed_INVITE",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Server::a6_completed_completed_INVITE,
            s_completed, s_completed);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_completed_confirmed_ACK",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Server::a7_completed_confirmed_ACK,
            s_completed, s_confirmed);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_completed_completed__timerG",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Server::a8_completed_completed_timerG,
            s_completed, s_completed);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_completed_terminated_errOrTimerH",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Server::a9_completed_terminated_errOrTimerH,
            s_completed, s_terminated);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_confirmed_terminated_timerI",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Server::a10_confirmed_terminated_timerI,
            s_confirmed, s_terminated);

    new State_Transition<Sip_SMCommand,std::string>(this, "a20_proceeding_proceeding__timer_rel1xx_resend",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Server::a20_proceeding_proceeding_timerRel1xxResend,
            s_proceeding, s_proceeding);

    set_current_state(s_start);
}

Sip_Transaction_Invite_Server::Sip_Transaction_Invite_Server(SRef<Sip_Stack_Internal*> stack_internal,
        int cseq,
        const std::string &cseq_method,
        const std::string &branch,
        const std::string &callid)
    :Sip_Transaction_Server(stack_internal, cseq, cseq_method, branch, callid),
      _last_response(NULL),
      _timerG(500)
{
    set_up_state_machine();
}

Sip_Transaction_Invite_Server::~Sip_Transaction_Invite_Server()
{
}

void Sip_Transaction_Invite_Server::set_dialog_route_set(SRef<Sip_Request*> inv)
{
}
