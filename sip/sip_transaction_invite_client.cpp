#include "sip_transaction_invite_client.h"
#include "sip_transition_utils.h"
#include "sip_command_dispatcher.h"
#include "sip_stack_internal.h"
#include "sip_command_string.h"
#include "sip_response.h"

bool Sip_Transaction_Invite_Client::a0_start_calling_INVITE( const Sip_SMCommand &command)
{
    if (transition_match("INVITE", command, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer))
    {
        _last_invite = (Sip_Request*) *command.get_command_packet();
        if( is_unreliable() )
        { // retx timer
            _timerA = _sip_stack_internal->get_timers()->getA();
            request_timeout( _timerA , "timerA" );
        }

        request_timeout( _sip_stack_internal->get_timers()->getB(), "timerB" ); //transaction timeout

        send( command.get_command_packet(), true ); // add via header
        return true;
    }
    return false;
}

bool Sip_Transaction_Invite_Client::a1_calling_calling_timerA( const Sip_SMCommand &command)
{
    if( transition_match(command, "timerA", Sip_SMCommand::transaction_layer, Sip_SMCommand::transaction_layer) )
    {
        _timerA *= 2; //no upper limit ... well ... timer B sets it
        request_timeout( _timerA, "timerA" );

        send( SRef<Sip_Message*>((Sip_Message*)* _last_invite), false );

        return true;
    }
    return false;
}

bool Sip_Transaction_Invite_Client::a2_calling_proceeding_1xx( const Sip_SMCommand &command)
{
    if( transition_match(Sip_Response::type, command, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer, "1**") )
    {

        cancel_timeout("timerA");
        cancel_timeout("timerB");
        Sip_SMCommand cmd( command.get_command_packet(), Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer);
        _dispatcher->enqueue_command( cmd, HIGH_PRIO_QUEUE );

        SRef<Sip_Response*> resp =(Sip_Response *)*command.get_command_packet();

        //assert(dialog);
        //TODO/XXX/FIXME: Do this in the TU instead
        //dialog->dialogState.updateState( resp );

        //rel1xxProcessing(resp);

        return true;
    }
    return false;
}

bool Sip_Transaction_Invite_Client::a3_calling_completed_resp36( const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer,
                         "3**\n4**\n5**\n6**"))
    {

        SRef<Sip_Response*> resp( (Sip_Response*) *command.get_command_packet() );

        cancel_timeout("timerA");
        cancel_timeout("timerB");
        if( is_unreliable() )
            request_timeout( _sip_stack_internal->get_timers()->getD(),"timerD" );
        else
            request_timeout( 0,"timerD");
        Sip_SMCommand cmd( command.get_command_packet(), Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer);
        _dispatcher->enqueue_command( cmd, HIGH_PRIO_QUEUE );

        //assert(dialog);
        //TODO/XXX/FIXME: Do this in the TU instead
        //dialog->dialogState.updateState( resp );

        send_ack(resp);
        return true;
    }
    return false;
}

bool Sip_Transaction_Invite_Client::a4_calling_terminated_ErrOrTimerB( const Sip_SMCommand &command)
{
    if ( transition_match(command, Sip_Command_String::transport_error, Sip_SMCommand::transport_layer,
                         Sip_SMCommand::transaction_layer)
         || transition_match(command, "timerB", Sip_SMCommand::transaction_layer, Sip_SMCommand::transaction_layer) )
    {
        cancel_timeout("timerA");
        cancel_timeout("timerB");

        Command_String terr( _call_id, Sip_Command_String::transport_error) ;
        terr["tid"] = get_transaction_id();

        Sip_SMCommand cmd( terr, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer );

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

bool Sip_Transaction_Invite_Client::a5_calling_terminated_2xx( const Sip_SMCommand &command)
{
    if( transition_match(Sip_Response::type, command, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer, "2**") )
    {
        cancel_timeout("timerA");
        cancel_timeout("timerB");

        Sip_SMCommand cmd( command.get_command_packet(), Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer);
        _dispatcher->enqueue_command( cmd, HIGH_PRIO_QUEUE );


        //FIXME/XXX/TODO: Implement setDialogRouteSet in TU instead
        //-EE

        //update dialogs route set ... needed to add route headers to the ACK we are going to send
        //setDialogRouteSet( (Sip_Response*)*command.get_command_packet() );

        //assert(dialog);
        //dialog->dialogState.updateState( (SRef<Sip_Response*>((Sip_Response *)*command.get_command_packet()) ) );

        Sip_SMCommand cmdterminated(
                    Command_String( get_transaction_id(), Sip_Command_String::transaction_terminated),
                    Sip_SMCommand::transaction_layer,
                    Sip_SMCommand::dispatcher);
        _dispatcher->enqueue_command( cmdterminated, HIGH_PRIO_QUEUE );

        return true;
    }
    return false;
}

bool Sip_Transaction_Invite_Client::a6_proceeding_proceeding_1xx( const Sip_SMCommand &command)
{
    if( transition_match(Sip_Response::type, command, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer, "1**") )
    {
        Sip_SMCommand cmd( command.get_command_packet(), Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer);

        SRef<Sip_Response*> resp = (Sip_Response *)*command.get_command_packet();

        _dispatcher->enqueue_command( cmd, HIGH_PRIO_QUEUE );

        //assert(dialog);
        //TODO/XXX/FIXME: Do this in the TU instead
        //dialog->dialogState.updateState( resp );

        //rel1xxProcessing(resp);
        return true;
    }
    return false;
}

bool Sip_Transaction_Invite_Client::a7_proceeding_terminated_2xx( const Sip_SMCommand &command)
{
    if( transition_match(Sip_Response::type, command, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer, "2**") )
    {
        cancel_timeout("timerA");
        cancel_timeout("timerB");

        Sip_SMCommand cmd( command.get_command_packet(), Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer);
        _dispatcher->enqueue_command( cmd, HIGH_PRIO_QUEUE );

        //update dialogs route set ... needed to add route headers to the ACK we are going to send
        //setDialogRouteSet( (Sip_Response*)*command.get_command_packet() );
        //assert(dialog);
        //TODO/XXX/FIXME: In Tu instead
        //dialog->dialogState.updateState( (SRef<Sip_Response*>((Sip_Response *)*command.get_command_packet()) ) );

        Sip_SMCommand cmdterminated(
                    Command_String( get_transaction_id(), Sip_Command_String::transaction_terminated),
                    Sip_SMCommand::transaction_layer,
                    Sip_SMCommand::dispatcher);
        _dispatcher->enqueue_command( cmdterminated, HIGH_PRIO_QUEUE );

        return true;
    }
    return false;
}

bool Sip_Transaction_Invite_Client::a8_proceeding_completed_resp36( const Sip_SMCommand &command)
{
    if( transition_match(Sip_Response::type, command, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer,
                        "3**\n4**\n5**\n6**") )
    {
        SRef<Sip_Response *> resp((Sip_Response*)*command.get_command_packet());
        cancel_timeout("timerA");
        cancel_timeout("timerB");
        if( is_unreliable() )
            request_timeout(_sip_stack_internal->get_timers()->getD(),"timerD");
        else
            request_timeout( 0,"timerD");

        Sip_SMCommand cmd( command.get_command_packet(), Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer);
        _dispatcher->enqueue_command( cmd, HIGH_PRIO_QUEUE );

        //assert(dialog);
        //IN TU instead TODO/XXX/FIXME
        //dialog->dialogState.updateState( resp );

        send_ack(resp);
        return true;
    }
    return false;
}

bool Sip_Transaction_Invite_Client::a9_completed_completed_resp36( const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::transport_layer, Sip_SMCommand::transaction_layer,
                        "3**\n4**\n5**\n6**"))
    {
        SRef<Sip_Response *> resp((Sip_Response*)*command.get_command_packet());
        send_ack(resp);
        return true;
    }
    return false;
}

bool Sip_Transaction_Invite_Client::a10_completed_terminated_TErr( const Sip_SMCommand &command)
{
    if(transition_match(command, Sip_Command_String::transport_error, Sip_SMCommand::transaction_layer,
                       Sip_SMCommand::transaction_layer))
    {

        cancel_timeout("timerD");

        Sip_SMCommand cmd( Command_String( _call_id, Sip_Command_String::transport_error),
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

bool Sip_Transaction_Invite_Client::a11_completed_terminated_timerD( const Sip_SMCommand &command)
{
    if (transition_match(command,  "timerD", Sip_SMCommand::transaction_layer, Sip_SMCommand::transaction_layer))
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


Sip_Transaction_Invite_Client::Sip_Transaction_Invite_Client(SRef<Sip_Stack_Internal*> stack_internal,
                                                             int cseq,
                                                             const std::string &cseq_method,
                                                             const std::string &callid)
    :Sip_Transaction_Client(stack_internal, cseq, cseq_method, "", callid),
      _last_invite(NULL)
{
    _timerA = _sip_stack_internal->get_timers()->getA();
    set_up_state_machine();
}

Sip_Transaction_Invite_Client::~Sip_Transaction_Invite_Client()
{
}


void Sip_Transaction_Invite_Client::set_up_state_machine()
{
    State<Sip_SMCommand,std::string> *s_start=new State<Sip_SMCommand,std::string>(this,"start");
    add_state(s_start);

    State<Sip_SMCommand,std::string> *s_calling=new State<Sip_SMCommand,std::string>(this,"calling");
    add_state(s_calling);

    State<Sip_SMCommand,std::string> *s_proceeding=new State<Sip_SMCommand,std::string>(this,"proceeding");
    add_state(s_proceeding);

    State<Sip_SMCommand,std::string> *s_completed=new State<Sip_SMCommand,std::string>(this,"completed");
    add_state(s_completed);

    State<Sip_SMCommand,std::string> *s_terminated=new State<Sip_SMCommand,std::string>(this,"terminated");
    add_state(s_terminated);

    //Set up cancel transitions
    new State_Transition<Sip_SMCommand,std::string>(this, "transition_cancel_transaction",
                                             (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                                             &Sip_Transaction::a1000_anyState_terminated_canceltransaction,
                                             State_Machine<Sip_SMCommand,std::string>::any_state, s_terminated);
    //
    new State_Transition<Sip_SMCommand,std::string>(this, "transition_start_calling_INVITE",
                                             (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Client::a0_start_calling_INVITE,
                                             s_start, s_calling);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_calling_calling_timerA",
                                             (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Client::a1_calling_calling_timerA,
                                             s_calling, s_calling);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_calling_proceeding_1xx",
                                             (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Client::a2_calling_proceeding_1xx,
                                             s_calling, s_proceeding);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_calling_completed_resp36",
                                             (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Client::a3_calling_completed_resp36,
                                             s_calling, s_completed);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_calling_terminated_ErrOrTimerB",
                                             (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Client::a4_calling_terminated_ErrOrTimerB,
                                             s_calling, s_terminated);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_calling_terminated_2xx",
                                             (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Client::a5_calling_terminated_2xx,
                                             s_calling, s_terminated);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_proceeding_proceeding_1xx",
                                             (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Client::a6_proceeding_proceeding_1xx,
                                             s_proceeding, s_proceeding);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_proceeding_terminated_2xx",
                                             (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Client::a7_proceeding_terminated_2xx,
                                             s_proceeding, s_terminated);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_proceeding_completed_resp36",
                                             (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Client::a8_proceeding_completed_resp36,
                                             s_proceeding, s_completed);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_completed_completed_resp36",
                                             (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Client::a9_completed_completed_resp36,
                                             s_completed, s_completed);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_completed_terminated_TErr",
                                             (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Client::a10_completed_terminated_TErr,
                                             s_completed, s_terminated);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_completed_terminated_timerD",
                                             (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Client::a11_completed_terminated_timerD,
                                             s_completed, s_terminated);

    set_current_state(s_start);
}

void Sip_Transaction_Invite_Client::set_dialog_route_set(SRef<Sip_Response *> resp)
{

}

void Sip_Transaction_Invite_Client::send_ack(SRef<Sip_Response *> resp, bool provisional)
{
    SRef<Sip_Request*> ack= Sip_Request::create_sip_message_ack( _last_invite, resp, provisional );

    if (provisional)
    {
        //TODO/XXX/FIXME: What is this needed for? --EE
        //int seq = dialog->dialogState.seqNo++;
        //((Sip_Header_Value_CSeq*)*ack->get_header_value_no(SIP_HEADER_TYPE_CSEQ, 0))->setCSeq(seq);
    }

    send(SRef<Sip_Message*>(*ack), true, _last_invite->get_branch() );
}
