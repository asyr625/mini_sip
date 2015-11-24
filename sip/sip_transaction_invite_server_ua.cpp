#include <iostream>

#include "sip_transaction_invite_server_ua.h"
#include "sip_transition_utils.h"
#include "sip_command_dispatcher.h"
#include "sip_command_string.h"
#include "sip_stack_internal.h"

bool Sip_Transaction_Invite_Server_Ua::a1001_proceeding_completed_2xx( const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer, "2**"))
    {
        cancel_timeout("timerRel1xxResend");
        _last_response = SRef<Sip_Response*>((Sip_Response*)*command.get_command_packet());

        if( is_unreliable() )
        {
            _timerG = _sip_stack_internal->get_timers()->getG();
            request_timeout(_timerG, "timerG");
        }
        request_timeout(_sip_stack_internal->get_timers()->getH(),"timerH");

        send(command.get_command_packet(), false);
        return true;
    }
    return false;
}

void Sip_Transaction_Invite_Server_Ua::change_state_machine()
{
    SRef<State<Sip_SMCommand, std::string> *> s_proceeding = get_state("proceeding");
    my_assert(s_proceeding);

    bool success = s_proceeding->remove_transition("transition_proceeding_terminated_2xx");
    if (!success)
    {
        my_err << "ERROR: Could not remove transition from state machine in Sip_Transaction_Invite_ServerUA (BUGBUG!!)"<<std::endl;
        my_assert(0==1);
    }

    SRef<State<Sip_SMCommand, std::string> *>s_completed = get_state("completed");
    my_assert(s_completed);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_proceeding_completed_2xx",
            (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Transaction_Invite_Server_Ua::a1001_proceeding_completed_2xx,
            s_proceeding, s_completed);
}

Sip_Transaction_Invite_Server_Ua::Sip_Transaction_Invite_Server_Ua(SRef<Sip_Stack_Internal*> stack_internal,
        int cseq,
        const std::string &cseq_method,
        const std::string &branch,
        const std::string &callid)
    : Sip_Transaction_Invite_Server(stack_internal, cseq, cseq_method, branch, callid)
{
    change_state_machine();
}

Sip_Transaction_Invite_Server_Ua::~Sip_Transaction_Invite_Server_Ua()
{
}
