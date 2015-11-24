#include <iostream>

#include "sip_dialog_management.h"

#include "sip_command_string.h"
#include "sip_transition_utils.h"

#define SHUTDOWN_CALLS_TIMEOUT 3000
#define SHUTDOWN_DEREGISTER_TIMEOUT 3000

bool Sip_Dialog_Management::a0_start_startShutdown_startShutdown( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::sip_stack_shutdown, Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer))
    {
        pending_hang_ups = pending_de_regs = 0;
        my_err << std::endl;
        my_err << "MiniSIP's SipStack is shutting down ... " << std::endl;
        my_err << "     ... it won't take long to finish, be patient. Thanks!" << std::endl;
        Sip_SMCommand cmd( Command_String( "", Sip_Command_String::terminate_all_calls),
                           Sip_SMCommand::dispatcher,
                           Sip_SMCommand::dispatcher);
        get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE);
        return true;
    }
    return false;
}

bool Sip_Dialog_Management::a10_startSh_terminateCallsSh_terminateAll( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::terminate_all_calls, Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer))
    {
        terminate_all_calls();
        request_timeout( SHUTDOWN_CALLS_TIMEOUT, "timer_terminate_calls" );
        return true;
    }
    return false;
}

bool Sip_Dialog_Management::a11_terminateCallsSh_callTerminatedEarly( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::call_terminated_early, Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer) )
    {
        received_call_terminate_early();
        //my_dbg << "shutdown: call terminated early: " << command.getCommand_String().get_destination_id() << end;
        return true;
    }
    return false;
}

bool Sip_Dialog_Management::a12_terminateCallsSh_timeIsUp( const Sip_SMCommand &command)
{
    if (transition_match(command, "timer_terminate_calls", Sip_SMCommand::dialog_layer,
                         Sip_SMCommand::dialog_layer) )
    {
        Sip_SMCommand cmd( Command_String( "", Sip_Command_String::unregister_all_identities),
                           Sip_SMCommand::dispatcher,
                           Sip_SMCommand::dispatcher);
        get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE);
        return true;
    }
    return false;
}

bool Sip_Dialog_Management::a13_terminateCallsSh_allTerminated( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::terminate_all_calls_done, Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer) )
    {
        cancel_timeout( "timer_terminate_calls" );
        Sip_SMCommand cmd( Command_String( "", Sip_Command_String::unregister_all_identities),
                           Sip_SMCommand::dispatcher,
                           Sip_SMCommand::dispatcher);
        get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE);
        return true;
    }
    return false;
}

bool Sip_Dialog_Management::a20_terminateCallsSh_deRegAllSh_allTerminated( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::unregister_all_identities, Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer) )
    {
        request_timeout( SHUTDOWN_DEREGISTER_TIMEOUT, "timer_de_register_all" );
        de_register_all();
        return true;
    }
    return false;
}

bool Sip_Dialog_Management::a21_deRegAllSh_callTerminatedEarly( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::call_terminated_early, Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer) )
    {
        received_call_terminate_early();
        return true;
    }
    return false;
}

bool Sip_Dialog_Management::a22_deRegAllSh_registerOk( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::register_ok, Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer) )
    {
        received_register_ok(true); //we are deregistering ...
        return true;
    }
    return false;
}

bool Sip_Dialog_Management::a23_deRegAllSh_timeIsUp( const Sip_SMCommand &command)
{
    if (transition_match(command, "timer_de_register_all", Sip_SMCommand::dialog_layer,
                         Sip_SMCommand::dialog_layer) )
    {
        shutdown_done( true ); //force shutdown done message
        return true;
    }
    return false;
}

bool Sip_Dialog_Management::a24_deRegAllSh_deRegAlldone( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::unregister_all_identities_done, Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer) )
    {
        shutdown_done( false ); //check if finished ... don't force
        return true;
    }
    return false;
}

bool Sip_Dialog_Management::a25_deRegAllSh_allTerminated( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::terminate_all_calls_done, Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer) )
    {
        shutdown_done( false ); //check if finished ... don't force
        return true;
    }
    return false;
}

bool Sip_Dialog_Management::a30_deRegAllSh_terminated_timeIsUp( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::sip_stack_shutdown_done, Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer) )
        return true;
    return false;
}

bool Sip_Dialog_Management::b0_start_terminateCallsOps_terminateAll( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::terminate_all_calls, Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer))
    {
        pending_hang_ups = pending_de_regs = 0;
        terminate_all_calls();
        request_timeout( SHUTDOWN_CALLS_TIMEOUT, "timer_terminate_calls" );
        return true;
    }
    return false;
}

bool Sip_Dialog_Management::b11_terminateCallsOps_terminateEarly( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::call_terminated_early, Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer) )
    {
        received_call_terminate_early();
        //my_dbg << "shutdown: call terminated early: " << command.getCommand_String().get_destination_id() << end;
        return true;
    }
    return false;
}

bool Sip_Dialog_Management::b12_terminateCallsOps_timeIsUp( const Sip_SMCommand &command)
{
    if (transition_match(command, "timer_terminate_calls", Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer) )
    {
        Sip_SMCommand cmd( Command_String( "", Sip_Command_String::unregister_all_identities),
                           Sip_SMCommand::dispatcher,
                           Sip_SMCommand::dispatcher);
        get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE);
        return true;
    }
    return false;
}

bool Sip_Dialog_Management::b30_terminateCallsOps_start_terminateAllDone( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::terminate_all_calls_done, Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer) )
    {
        cancel_timeout( "timer_terminate_calls" );
        return true;
    }
    return false;
}

bool Sip_Dialog_Management::c0_start_deRegAllOps_deRegAll( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::unregister_all_identities, Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer) )
    {
        pending_hang_ups = pending_de_regs = 0;
        request_timeout( SHUTDOWN_DEREGISTER_TIMEOUT, "timer_de_register_all" );
        de_register_all();
        return true;
    }
    return false;
}

bool Sip_Dialog_Management::c11_deRegAllOps_registerOk( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::register_ok, Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer) )
    {
        received_register_ok(true); //we are deregistering ...
        return true;
    }
    return false;
}

bool Sip_Dialog_Management::c12_deRegAllOps_timeIsUp( const Sip_SMCommand &command)
{
    if (transition_match(command, "timer_de_register_all", Sip_SMCommand::dispatcher, Sip_SMCommand::dialog_layer) )
        return true;
    return false;
}

bool Sip_Dialog_Management::c30_deRegAllOps_start_deRegAllDone( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::unregister_all_identities_done, Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer) )
    {
        cancel_timeout( "timer_de_register_all" );
        return true;
    }
    return false;
}


bool Sip_Dialog_Management::d0_start_regAllOps_regAll( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::register_all_identities, Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer) )
    {
        pending_hang_ups = pending_de_regs = 0;
        request_timeout( SHUTDOWN_DEREGISTER_TIMEOUT, "timer_registerAll" );
        register_all();
        return true;
    }
    return false;
}

bool Sip_Dialog_Management::d11_regAllOps_registerOk( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::register_ok, Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer) )
    {
        received_register_ok(false); //we are NOT deregistering ...
        return true;
    }
    return false;
}

bool Sip_Dialog_Management::d12_regAllOps_timeIsUp( const Sip_SMCommand &command)
{
    if (transition_match(command, "timer_registerAll", Sip_SMCommand::dispatcher, Sip_SMCommand::dialog_layer) )
        return true;

    return false;
}

bool Sip_Dialog_Management::d30_regAllOps_start_regAllDone( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::register_all_identities_done, Sip_SMCommand::dispatcher,
                         Sip_SMCommand::dialog_layer) )
    {
        cancel_timeout( "timer_registerAll" );
        return true;
    }
    return false;
}

void Sip_Dialog_Management::set_up_state_machine()
{
    State<Sip_SMCommand,std::string> *s_start=
            new State<Sip_SMCommand,std::string>(this,"start");
    add_state(s_start);

    set_up_state_machine_shutdown(s_start);
    set_up_state_machine_dialogops(s_start);

    set_current_state(s_start);
}

void Sip_Dialog_Management::set_up_state_machine_shutdown(State<Sip_SMCommand,std::string> *s_start)
{
    State<Sip_SMCommand,std::string> *s_start_sh=
            new State<Sip_SMCommand,std::string>(this,"start_shutdown");
    add_state(s_start_sh);

    State<Sip_SMCommand,std::string> *s_terminateCalls_sh=
            new State<Sip_SMCommand,std::string>(this,"terminateCalls_shutdown");
    add_state(s_terminateCalls_sh);

    State<Sip_SMCommand,std::string> *s_deRegAll_sh=
            new State<Sip_SMCommand,std::string>(this,"de_register_all_shutdown");
    add_state(s_deRegAll_sh);

    State<Sip_SMCommand,std::string> *s_terminated=
            new State<Sip_SMCommand,std::string>(this,"terminated");
    add_state(s_terminated);


    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_start_startSh_startShutdown",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::a0_start_startShutdown_startShutdown,
                s_start,
                s_start_sh);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_startSh_terminateCallsSh_startShutdown",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::a10_startSh_terminateCallsSh_terminateAll,
                s_start_sh,
                s_terminateCalls_sh);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_terminateCallsSh_callTerminatedEarly",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::a11_terminateCallsSh_callTerminatedEarly,
                s_terminateCalls_sh,
                s_terminateCalls_sh);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_terminateCallsSh_timeIsUp",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::a12_terminateCallsSh_timeIsUp,
                s_terminateCalls_sh,
                s_terminateCalls_sh);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_terminateCallsSh_allTerminated",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::a13_terminateCallsSh_allTerminated,
                s_terminateCalls_sh,
                s_terminateCalls_sh);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_terminateCallsSh_s_deRegAllSh_allTerminated",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::a20_terminateCallsSh_deRegAllSh_allTerminated,
                s_terminateCalls_sh,
                s_deRegAll_sh);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_deRegAllSh_deRegAllSh_callTerminatedEarly",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::a21_deRegAllSh_callTerminatedEarly,
                s_deRegAll_sh,
                s_deRegAll_sh);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_deRegAllSh_deRegAllSh_registerOk",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::a22_deRegAllSh_registerOk,
                s_deRegAll_sh,
                s_deRegAll_sh);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_deRegAllSh_deRegAllSh_timeIsUp",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::a23_deRegAllSh_timeIsUp,
                s_deRegAll_sh,
                s_deRegAll_sh);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_deRegAllSh_deRegAllSh_deRegAlldone",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::a24_deRegAllSh_deRegAlldone,
                s_deRegAll_sh,
                s_deRegAll_sh);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_deRegAllSh_deRegAllSh_allTerminated",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::a25_deRegAllSh_allTerminated,
                s_deRegAll_sh,
                s_deRegAll_sh);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_deRegAllSh_terminated_shutdown_done",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::a30_deRegAllSh_terminated_timeIsUp,
                s_deRegAll_sh,
                s_terminated);
}

void Sip_Dialog_Management::set_up_state_machine_dialogops(State<Sip_SMCommand,std::string> *s_start)
{
    State<Sip_SMCommand,std::string> *s_terminateCalls_ops=
            new State<Sip_SMCommand,std::string>(this,"terminateCalls_ops");
    add_state(s_terminateCalls_ops);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_start_terminateCallsOps_terminateAll",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::b0_start_terminateCallsOps_terminateAll,
                s_start,
                s_terminateCalls_ops);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_terminateCallsOps_terminateEarly",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::b11_terminateCallsOps_terminateEarly,
                s_terminateCalls_ops,
                s_terminateCalls_ops);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_terminateCallsOps_timeIsUp",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::b12_terminateCallsOps_timeIsUp,
                s_terminateCalls_ops,
                s_terminateCalls_ops);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_terminateCallsOps_start_terminateAllDone",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::b30_terminateCallsOps_start_terminateAllDone,
                s_terminateCalls_ops,
                s_start);

    //UNREGISTER ALL IDENTITIES SETUP
    State<Sip_SMCommand,std::string> *s_deRegAll_ops=
            new State<Sip_SMCommand,std::string>(this,"deRegAll_ops");
    add_state(s_deRegAll_ops);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_start_deRegAllOps_terminateAll",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::c0_start_deRegAllOps_deRegAll,
                s_start,
                s_deRegAll_ops);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_deRegAllOps_registerOk",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::c11_deRegAllOps_registerOk,
                s_deRegAll_ops,
                s_deRegAll_ops);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_deRegAllOps_timeIsUp",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::c12_deRegAllOps_timeIsUp,
                s_deRegAll_ops,
                s_deRegAll_ops);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_deRegAllOps_start_terminateAllDone",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::c30_deRegAllOps_start_deRegAllDone,
                s_deRegAll_ops,
                s_start);

    //REGISTER ALL IDENTITIES SETUP
    State<Sip_SMCommand,std::string> *s_regAll_ops=
            new State<Sip_SMCommand,std::string>(this,"regAll_ops");
    add_state(s_regAll_ops);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_start_deRegAllOps_terminateAll",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::d0_start_regAllOps_regAll,
                s_start,
                s_regAll_ops);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_deRegAllOps_registerOk",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::d11_regAllOps_registerOk,
                s_regAll_ops,
                s_regAll_ops);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_deRegAllOps_timeIsUp",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::d12_regAllOps_timeIsUp,
                s_regAll_ops,
                s_regAll_ops);

    new State_Transition<Sip_SMCommand,std::string>(
                this,
                "transition_deRegAllOps_start_terminateAllDone",
                (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&))
                &Sip_Dialog_Management::d30_regAllOps_start_regAllDone,
                s_regAll_ops,
                s_start);
}

Sip_Dialog_Management::Sip_Dialog_Management(SRef<Sip_Stack*> stack)
    : Sip_Dialog(stack, NULL, "shutdown_dialog")
{
    set_up_state_machine();
    pending_hang_ups = 0;
    pending_de_regs = 0;
}

Sip_Dialog_Management::~Sip_Dialog_Management()
{
}


bool Sip_Dialog_Management::terminate_all_calls()
{
    std::list<SRef<Sip_Dialog *> > dlgs;
    dlgs = get_sip_stack()->get_dialogs();

    my_err << std::endl;
    my_err << "Terminating all ongoing calls:" << std::endl;
    for( std::list<SRef<Sip_Dialog *> >::iterator it = dlgs.begin();
         it != dlgs.end();
         it ++  ) {
        //First, skip the register dialogs ... we'll deal with them later
        if( (*it)->get_mem_object_type() == "SipDialogRegister" )
        {
            //my_dbg << "Sip_Dialog_Management::terminate_all_calls : dialog skipped" << end;
            continue;
        }
        Sip_SMCommand cmd( Command_String( (*it)->_dialog_state._call_id, Sip_Command_String::hang_up),
                           Sip_SMCommand::dialog_layer,
                           Sip_SMCommand::dialog_layer);
        get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE);
        pending_hang_ups++;
        my_err << "    - Hanging up " << (*it)->_dialog_state._remote_uri << std::endl;
    }
    if( pending_hang_ups <= 0 ) {
        my_err << "    CALLS: No ongoing calls!" << std::endl;
        //if we have not sent any hang_ups ... notify all calls terminated
        Sip_SMCommand cmd( Command_String( "", Sip_Command_String::terminate_all_calls_done),
                           Sip_SMCommand::dispatcher,
                           Sip_SMCommand::dispatcher);
        get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE);
    }
    return true;
}

bool Sip_Dialog_Management::received_call_terminate_early()
{
    pending_hang_ups --;
    if( pending_hang_ups <= 0 ) {
        my_err << "    CALLS: all calls have been terminated!" << std::endl;
        Sip_SMCommand cmd( Command_String( "", Sip_Command_String::terminate_all_calls_done),
                           Sip_SMCommand::dispatcher,
                           Sip_SMCommand::dispatcher);
        get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE);
    }
    return true;
}

bool Sip_Dialog_Management::de_register_all()
{
    std::list<SRef<Sip_Dialog *> > dlgs;
    dlgs = get_sip_stack()->get_dialogs();
    my_err << std::endl;
    my_err << "De-Registering all identities from their registrar:" << std::endl;
    for( std::list<SRef<Sip_Dialog *> >::iterator it = dlgs.begin();
         it != dlgs.end();
         it ++  ) {
        //First, skip the register dialogs ... we'll deal with them later
        if( (*it)->get_mem_object_type() != "SipDialogRegister" )
        {
            //my_dbg << "Sip_Dialog_Management::de_register_all : non-reg dialog skipped" << end;
            continue;
        }
        if(! (*it)->get_dialog_config()->_sip_identity->is_registered() )
        {
            //my_dbg << "Sip_Dialog_Management::de_register_all : skipping already de-registered identity" << end;
            continue;
        }

        Command_String cmdstr( (*it)->_dialog_state._call_id, Sip_Command_String::proxy_register);
        cmdstr["proxy_domain"] = (*it)->get_dialog_config()->_sip_identity->get_sip_uri().get_ip();
        cmdstr.set_param3("0"); //expires = 0 ==> de-register

        Sip_SMCommand cmd( cmdstr,
                           Sip_SMCommand::dialog_layer,
                           Sip_SMCommand::dialog_layer);
        get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE);
        pending_de_regs++;
        my_err << "    De-registration request sent (username = " <<
                  (*it)->get_dialog_config()->_sip_identity->get_sip_uri().get_string() << ")" << std::endl;
    }
    if( pending_de_regs == 0 ) {
        //if we have not sent any de-regs ... notify all un-registered
        my_err << "    DE-REGISTER: all identities were already not registered!" << std::endl;
        Sip_SMCommand cmd( Command_String( "", Sip_Command_String::unregister_all_identities_done),
                           Sip_SMCommand::dispatcher,
                           Sip_SMCommand::dispatcher);
        get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE);
    }
    return true;
}

bool Sip_Dialog_Management::register_all()
{
    std::list<SRef<Sip_Dialog *> > dlgs;
    dlgs = get_sip_stack()->get_dialogs();
    my_err << std::endl;
    my_err << "Registering all identities to their registrar:" << std::endl;
    for( std::list<SRef<Sip_Dialog *> >::iterator it = dlgs.begin();
         it != dlgs.end();
         it ++  ) {
        //First, skip the register dialogs ... we'll deal with them later
        if( (*it)->get_mem_object_type() != "SipDialogRegister" )
        {
            //my_dbg << "Sip_Dialog_Management::register_all : non-reg dialog skipped" << end;
            continue;
        }
        if( (*it)->get_dialog_config()->_sip_identity->is_registered() )
        {
            //my_dbg << "Sip_Dialog_Management::register_all : skipping already registered identity" << end;
            continue;
        }

        Command_String cmdstr( (*it)->_dialog_state._call_id, Sip_Command_String::proxy_register);
        cmdstr["proxy_domain"] = (*it)->get_dialog_config()->_sip_identity->get_sip_uri().get_ip();
        //expires = defaultExpires, read from the config file
        cmdstr.set_param3((*it)->get_dialog_config()->_sip_identity->get_sip_registrar()->get_default_expires());

        Sip_SMCommand cmd( cmdstr,
                           Sip_SMCommand::dialog_layer,
                           Sip_SMCommand::dialog_layer);
        get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE);
        pending_de_regs++;
        my_err << "    Registration request sent (username = " <<
                  (*it)->get_dialog_config()->_sip_identity->get_sip_uri().get_string() << ")" << std::endl;
    }
    if( pending_de_regs == 0 )
    {
        //if we have not sent any de-regs ... notify all un-registered
        my_err << "    REGISTER: all identities were already registered!" << std::endl;
        Sip_SMCommand cmd( Command_String( "", Sip_Command_String::unregister_all_identities_done),
                           Sip_SMCommand::dispatcher,
                           Sip_SMCommand::dispatcher);
        get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE);
    }
    return true;
}

bool Sip_Dialog_Management::received_register_ok(bool deregistering)
{
    pending_de_regs--;
    if( pending_de_regs <= 0 )
    {
        if( deregistering ) {
            my_err << "    DE-REGISTER: all identities have been de-registered correctly!" << std::endl;
            Sip_SMCommand cmd( Command_String( "", Sip_Command_String::unregister_all_identities_done),
                               Sip_SMCommand::dispatcher,
                               Sip_SMCommand::dispatcher);
            get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE);
        } else {
            my_err << "    REGISTER: all identities have been registered correctly!" << std::endl;
            Sip_SMCommand cmd( Command_String( "", Sip_Command_String::register_all_identities_done),
                               Sip_SMCommand::dispatcher,
                               Sip_SMCommand::dispatcher);
            get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE);
        }
    }
    return true;
}

bool Sip_Dialog_Management::shutdown_done( bool force )
{
    if( !force )
    {
        if( pending_hang_ups > 0 || pending_de_regs > 0 )
        {
            //Still can wait a bit more ...
            //my_dbg << "CESC: shutdown: still not finished ... wait ... "<< end;
            return false;
        }
        //else ... shutdown ... nothing else to do ...
        my_err << std::endl << "SipStack Shutdown process is completed."<< std::endl;
    } else {
        my_err << "Shutdown process timed out (there was some problem): "<< std::endl;
        if( pending_hang_ups > 0 ) {
            my_err << "      CALLS: Not all calls could be correctly hung up."<< std::endl;
        }
        if( pending_de_regs > 0 ) {
            my_err << "      DE-REGISTER: Not all identities were correctly de-registered."<< std::endl;
        }
    }

    Sip_SMCommand cmd(
                Command_String( "", Sip_Command_String::sip_stack_shutdown_done),
                Sip_SMCommand::dispatcher,
                Sip_SMCommand::dispatcher);
    get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE);
    return true;
}
