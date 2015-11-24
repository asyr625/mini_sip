#include "sip_dialog_presence_client.h"
#include "sip_dialog_config.h"
#include "sip_transition_utils.h"
#include "sip_command_string.h"

#include "sip_header_subscription_state.h"
#include "sip_header_from.h"
#include "sip_header_to.h"

#include "string_utils.h"

#include <stdlib.h>
#include <iostream>
using namespace std;

void Sip_Dialog_Presence_Client::create_subscribe_client_transaction()
{
    ++_dialog_state._seq_no;
    send_subscribe();
}

bool Sip_Dialog_Presence_Client::a0_start_trying_presence(const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::start_presence_client, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer))
    {
#ifdef DEBUG_OUTPUT
        my_err << "Sip_Dialog_Presence_Client::a0: Presence _to_uri is: <"<< command.get_command_string().get_param()<< ">"<< endl;
#endif
        _to_uri = SRef<Sip_Identity*>( new Sip_Identity(command.get_command_string().get_param()) );
        create_subscribe_client_transaction();
        return true;
    }
    return false;
}

bool Sip_Dialog_Presence_Client::a1_X_subscribing_200OK(const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer, "2**"))
    {
        SRef<Sip_Response*> resp(  (Sip_Response*)*command.get_command_packet() );
        _dialog_state._remote_tag = command.get_command_packet()->get_header_value_to()->get_parameter("tag");

        SRef<Sip_Header_Value_Subscription_State *> statehdr = (Sip_Header_Value_Subscription_State*)*resp->get_header_value_no(SIP_HEADER_TYPE_SUBSCRIPTIONSTATE,0);

        int to;
        if( statehdr && statehdr->has_parameter("expires") )
        {
            to = atoi(statehdr->get_parameter("expires").c_str());
        }
        else
        {
            my_dbg("signaling/sip") << "WARNING: Sip_Dialog_Presence_Client did not contain any expires header - using 300 seconds"<<endl;
            to = 300;
        }

        request_timeout(to * 1000, "timerDoSubscribe");

#ifdef DEBUG_OUTPUT
        my_err << "Subscribed for presence for user "<< _to_uri->get_sip_uri().get_string()<< endl;
#endif
        return true;
    }
    return false;
}

bool Sip_Dialog_Presence_Client::a2_trying_retrywait_transperror(const Sip_SMCommand &command)
{
    if(transition_match(command, Sip_Command_String::transport_error, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer ))
    {
        my_dbg("signaling/sip") << "WARNING: Transport error when subscribing - trying again in five minutes"<< endl;
        request_timeout(300 * 1000, "timerDoSubscribe");
        return true;
    }
    return false;
}

bool Sip_Dialog_Presence_Client::a4_X_trying_timerTO(const Sip_SMCommand &command)
{
    if (transition_match(command, "timerDoSubscribe", Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer))
    {
        create_subscribe_client_transaction();
        return true;
    }
    return false;
}

bool Sip_Dialog_Presence_Client::a5_subscribing_subscribing_NOTIFY(const Sip_SMCommand &command)
{
    if (transition_match("NOTIFY", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer))
    {
        SRef<Sip_Request*> notify = dynamic_cast<Sip_Request*>(*command.get_command_packet());
        send_notify_ok(notify);

        Command_String cmdstr(_dialog_state._call_id, Sip_Command_String::remote_presence_update,"UNIMPLEMENTED_INFO");
        get_sip_stack()->get_callback()->handle_command("gui",cmdstr);
        return true;
    }
    return false;
}

bool Sip_Dialog_Presence_Client::a6_subscribing_termwait_stoppresence(const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::hang_up, Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer))
    {
        signal_if_no_transactions();
        return true;
    }
    return false;
}

bool Sip_Dialog_Presence_Client::a7_termwait_terminated_notransactions(const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::no_transactions, Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer) )
    {
        _dialog_state._is_terminated=true;

        Sip_SMCommand cmd( Command_String( _dialog_state._call_id, Sip_Command_String::call_terminated),
                           Sip_SMCommand::dialog_layer,
                           Sip_SMCommand::dispatcher);
        get_sip_stack()->enqueue_command( cmd, HIGH_PRIO_QUEUE );
        return true;
    }
    return false;
}

bool Sip_Dialog_Presence_Client::a8_trying_trying_40X(const Sip_SMCommand &command)
{
    if (transition_match_sip_response("SUBSCRIBE", command,Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer, "407\n401"))
    {
        SRef<Sip_Response*> resp( (Sip_Response*)*command.get_command_packet() );

        _dialog_state.update_state( resp ); //nothing will happen ... 4xx responses do not update ...

        ++_dialog_state._seq_no;

        if( !update_authentications( resp ) )
        {
            my_dbg("signaling/sip") << "Auth failed" << endl;
            return true;
        }
        send_subscribe();
        return true;
    }
    return false;
}

bool Sip_Dialog_Presence_Client::a9_trying_retry_wait_failure(const Sip_SMCommand &command)
{
    if (transition_match_sip_response("SUBSCRIBE", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer, "3**\n4**\n5**\n6**")){

        SRef<Sip_Response*> resp( (Sip_Response*)*command.get_command_packet() );

        _dialog_state.update_state( resp ); //nothing will happen ...

        ++_dialog_state._seq_no;
        return true;
    }
    return false;
}


void Sip_Dialog_Presence_Client::set_up_state_machine()
{
    State<Sip_SMCommand,std::string> *s_start = new State<Sip_SMCommand,std::string>(this,"start");
    add_state(s_start);

    State<Sip_SMCommand,std::string> *s_trying = new State<Sip_SMCommand,std::string>(this,"trying");
    add_state(s_trying);

    State<Sip_SMCommand,std::string> *s_retry_wait = new State<Sip_SMCommand,std::string>(this,"retry_wait");
    add_state(s_retry_wait);

    State<Sip_SMCommand,std::string> *s_subscribing = new State<Sip_SMCommand,std::string>(this,"subscribing");
    add_state(s_subscribing);

    State<Sip_SMCommand,std::string> *s_termwait=new State<Sip_SMCommand,std::string>(this,"termwait");
    add_state(s_termwait);

    State<Sip_SMCommand,std::string> *s_terminated=new State<Sip_SMCommand,std::string>(this,"terminated");
    add_state(s_terminated);


    new State_Transition<Sip_SMCommand,std::string>(this, "transition_start_trying_presence",
                                                    (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Presence_Client::a0_start_trying_presence,
                                                    s_start, s_trying);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_trying_subscribing_200OK",
                                                    (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Presence_Client::a1_X_subscribing_200OK,
                                                    s_trying, s_subscribing);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_trying_retrywait",
                                                    (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Presence_Client::a2_trying_retrywait_transperror,
                                                    s_trying, s_retry_wait);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_retrywait_subscribing_200OK",
                                                    (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Presence_Client::a1_X_subscribing_200OK,
                                                    s_retry_wait, s_subscribing);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_subscribing_trying_timerTO",
                                                    (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Presence_Client::a4_X_trying_timerTO,
                                                    s_subscribing, s_trying);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_retrywait_trying_timerTO",
                                                    (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Presence_Client::a4_X_trying_timerTO,
                                                    s_subscribing, s_trying);

    //Should add this/similar transition to the trying case (when
    //updating registration, we should be ready to receive a NOTIFY).
    new State_Transition<Sip_SMCommand,std::string>(this, "transition_subscribing_subscribing_NOTIFY",
                                                    (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Presence_Client::a5_subscribing_subscribing_NOTIFY,
                                                    s_subscribing, s_subscribing);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_subscribing_termwait_stoppresence",
                                                    (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Presence_Client::a6_subscribing_termwait_stoppresence,
                                                    s_subscribing, s_termwait);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_termwait_terminated_notransactions",
                                                    (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Presence_Client::a7_termwait_terminated_notransactions,
                                                    s_termwait, s_terminated);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_termwait_terminated_notransactions",
                                                    (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Presence_Client::a8_trying_trying_40X,
                                                    s_trying, s_trying);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_termwait_terminated_notransactions",
                                                    (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Presence_Client::a9_trying_retry_wait_failure,
                                                    s_trying, s_retry_wait);

    set_current_state(s_start);
}

Sip_Dialog_Presence_Client::Sip_Dialog_Presence_Client(SRef<Sip_Stack*> stack, SRef<Sip_Identity*> ident, bool use_stun)
    : Sip_Dialog(stack,ident, ""), _use_stun(use_stun)
{
    set_up_state_machine();
}

Sip_Dialog_Presence_Client::~Sip_Dialog_Presence_Client()
{
}

void Sip_Dialog_Presence_Client::send_subscribe()
{
    SRef<Sip_Request*> sub ;

    sub = Sip_Request::create_sip_message_subscribe(
                _dialog_state._call_id,
                _to_uri->get_sip_uri(),
                get_dialog_config()->_sip_identity->get_sip_uri(),
                get_dialog_config()->get_contact_uri(_use_stun),
                _dialog_state._seq_no
                );

    sub->get_header_value_from()->set_parameter("tag",_dialog_state._local_tag);

    add_authorizations( sub );
    add_route( sub );

    SRef<Sip_Message*> pktr(*sub);

    Sip_SMCommand scmd( pktr, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer );

    get_sip_stack()->enqueue_command(scmd, HIGH_PRIO_QUEUE );
}

void Sip_Dialog_Presence_Client::send_notify_ok(SRef<Sip_Request*> notify)
{
    SRef<Sip_Response*> ok= create_sip_response(notify, 200, "OK");

    SRef<Sip_Message*> pref(*ok);
    Sip_SMCommand cmd( pref, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE);
}

bool Sip_Dialog_Presence_Client::handle_command(const Sip_SMCommand &c)
{
    my_dbg("signaling/sip") << "Sip_Dialog_Presence_Client::handle_command got "<< c << endl;

    if (c.get_type()==Sip_SMCommand::COMMAND_STRING && _dialog_state._call_id.length()>0)
    {
        if (c.get_command_string().get_destination_id() != _dialog_state._call_id )
        {
            cerr << "Sip_Dialog_Presence_Client returning false based on _call_id"<< endl;
            return false;
        }
    }

    if (c.get_type()==Sip_SMCommand::COMMAND_PACKET  && _dialog_state._call_id.length()>0)
    {
        if (c.get_command_packet()->get_call_id() != _dialog_state._call_id )
        {
            return false;
        }
        if (c.get_type()!=Sip_SMCommand::COMMAND_PACKET &&
                c.get_command_packet()->get_cseq() != _dialog_state._seq_no)
        {
            return false;
        }

    }

    my_dbg("signaling/sip") << "Sip_Dialog_Presence_Client::handlePacket() got "<< c << endl;
    bool handled = Sip_Dialog::handle_command(c);

    if (!handled && c.get_type() == Sip_SMCommand::COMMAND_STRING && c.get_command_string().get_op() == Sip_Command_String::no_transactions)
    {
        return true;
    }

    if (c.get_type() == Sip_SMCommand::COMMAND_STRING && _dialog_state._call_id.length()>0)
    {
        if (c.get_command_string().get_destination_id() == _dialog_state._call_id )
        {
            my_dbg("signaling/sip") << "Warning: Sip_Dialog_Presence_Client ignoring command with matching call id"<< endl;
            return true;
        }
    }
    if (c.get_type() == Sip_SMCommand::COMMAND_PACKET && _dialog_state._call_id.length()>0)
    {
        if (c.get_command_packet()->get_call_id() == _dialog_state._call_id)
        {
            my_dbg("signaling/sip") << "Warning: Sip_Dialog_Presence_Client ignoring packet with matching call id"<< endl;
            return true;
        }
    }

    return handled;
}
