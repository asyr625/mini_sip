#include "sip_dialog_presence_server.h"
#include"presence_message_content.h"
#include "sip_command_string.h"
#include "sip_transition_utils.h"
#include "sip_header_to.h"
#include "sip_header_from.h"

#include "string_utils.h"
#include <stdlib.h>
#include <iostream>
using namespace std;

bool Sip_Dialog_Presence_Server::a0_start_default_startpresenceserver(const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::start_presence_server, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer))
    {
        return true;
    }
    return false;
}

bool Sip_Dialog_Presence_Server::a1_default_default_timerremovesubscriber(const Sip_SMCommand &command)
{
    if (command.get_type()==Sip_SMCommand::COMMAND_STRING &&
            command.get_command_string().get_op().substr(0,22)=="timy_erremoveSubscriber_")
    {

        std::string user = command.get_command_string().get_op().substr(22);
        cerr << "Removing user <"<< user << ">"<< endl;
        remove_user(user);
        return true;
    }
    return false;
}

bool Sip_Dialog_Presence_Server::a2_default_default_localpresenceupdated(const Sip_SMCommand &command)
{
    if (transition_match(command,
                         Sip_Command_String::local_presence_update,
                         Sip_SMCommand::dialog_layer,
                         Sip_SMCommand::dialog_layer))
    {
        _online_status = command.get_command_string().get_param();
        send_notice_to_all(command.get_command_string().get_param());
        return true;
    }
    return false;
}

bool Sip_Dialog_Presence_Server::a3_default_termwait_stoppresenceserver(const Sip_SMCommand &command)
{
    if (transition_match(command,
                         Sip_Command_String::stop_presence_server,
                         Sip_SMCommand::dialog_layer,
                         Sip_SMCommand::dialog_layer))
    {
        signal_if_no_transactions();
        return true;
    }
    return false;
}

bool Sip_Dialog_Presence_Server::a4_termwait_terminated_notransactions(const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::no_transactions, Sip_SMCommand::dialog_layer,
                         Sip_SMCommand::dialog_layer) )
    {

        _dialog_state._is_terminated = true;

        Sip_SMCommand cmd( Command_String( _dialog_state._call_id, Sip_Command_String::call_terminated), //FIXME: _call_id is ""
                           Sip_SMCommand::dialog_layer,
                           Sip_SMCommand::dispatcher);
        get_sip_stack()->enqueue_command( cmd, HIGH_PRIO_QUEUE);
        return true;
    }
    return false;
}

bool Sip_Dialog_Presence_Server::a5_default_default_SUBSCRIBE(const Sip_SMCommand &command)
{
    if (transition_match("SUBSCRIBE", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer)){
        SRef<Sip_Request *> sub = (Sip_Request*)*command.get_command_packet();

        std::string user = sub->get_header_value_to()->get_uri().get_user_ip_string();
        cerr <<"Sip_Dialog_Presence_Server::a5_default_default_SUBSCRIBE: got subscribe request from <"<<user<<">"<<endl;

        add_user(user);

        send_subscribe_ok(sub);
        return true;
    }
    return false;
}

void Sip_Dialog_Presence_Server::set_up_state_machine()
{
    State<Sip_SMCommand,std::string> *s_start = new State<Sip_SMCommand,std::string>(this,"start");
    add_state(s_start);

    State<Sip_SMCommand,std::string> *s_default = new State<Sip_SMCommand,std::string>(this,"default");
    add_state(s_default);

    State<Sip_SMCommand,std::string> *s_termwait = new State<Sip_SMCommand,std::string>(this,"termwait");
    add_state(s_termwait);

    State<Sip_SMCommand,std::string> *s_terminated = new State<Sip_SMCommand,std::string>(this,"terminated");
    add_state(s_terminated);


    new State_Transition<Sip_SMCommand,std::string>(this, "transition_start_default_startpresenceserver",
                                                    (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Presence_Server::a0_start_default_startpresenceserver,
                                                    s_start, s_default);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_default_default_timy_erremovesubscriber",
                                                    (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Presence_Server::a1_default_default_timerremovesubscriber,
                                                    s_default, s_default);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_default_default_localpresenceupdated",
                                                    (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Presence_Server::a2_default_default_localpresenceupdated,
                                                    s_default, s_default);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_default_termwait_stoppresenceserver",
                                                    (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Presence_Server::a3_default_termwait_stoppresenceserver,
                                                    s_default, s_termwait);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_termwait_terminated_notransactions",
                                                    (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Presence_Server::a4_termwait_terminated_notransactions,
                                                    s_termwait, s_terminated);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_default_default_SUBSCRIBE",
                                                    (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Presence_Server::a5_default_default_SUBSCRIBE,
                                                    s_default, s_default);


    set_current_state(s_start);
}

Sip_Dialog_Presence_Server::Sip_Dialog_Presence_Server(SRef<Sip_Stack*> dContainer, SRef<Sip_Identity*> ident, bool use_stun)
    : Sip_Dialog(dContainer,ident,""),
      _use_stun(use_stun),
      _online_status("online")
{
    set_up_state_machine();
}

Sip_Dialog_Presence_Server::~Sip_Dialog_Presence_Server()
{
}

void Sip_Dialog_Presence_Server::send_notice_to_all( std::string onlineStatus)
{
    _users_lock.lock();
    for (int i=0; i<_subscribing_users.size(); i++){
        send_notice(onlineStatus, _subscribing_users[i]);
    }
    _users_lock.unlock();
}

void Sip_Dialog_Presence_Server::send_notice( std::string onlinestatus,  std::string user)
{
    ++_dialog_state._seq_no;
    std::string cid = "FIXME"+itoa(rand());
    send_notify( user, cid );
}

void Sip_Dialog_Presence_Server::send_subscribe_ok(SRef<Sip_Request*> sub)
{
    SRef<Sip_Response*> ok= new Sip_Response( 200,"OK", sub );
    ok->get_header_value_to()->set_parameter("tag",_dialog_state._local_tag);

    SRef<Sip_Message*> pref(*ok);
    Sip_SMCommand cmd( pref, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE);

    send_notice(_online_status, sub->get_from().get_user_ip_string());
}

void Sip_Dialog_Presence_Server::remove_user( std::string user)
{
    _users_lock.lock();
    _subscribing_users.remove(user);
    _users_lock.unlock();
}

void Sip_Dialog_Presence_Server::add_user( std::string user)
{
    _users_lock.lock();
    _subscribing_users.push_back(user);
    _users_lock.unlock();
}

void Sip_Dialog_Presence_Server::send_notify(std::string toUri,  std::string cid)
{
    SRef<Sip_Request*> notify;
    int32_t localSipPort;

    localSipPort = get_sip_stack()->get_local_sip_port( _use_stun );

    SRef<Sip_Identity*> toId( new Sip_Identity(toUri));
    notify = Sip_Request::create_sip_message_notify(
                cid,
                toId->get_sip_uri(),
                get_dialog_config()->_sip_identity->get_sip_uri(),
                _dialog_state._seq_no
                );

    notify->get_header_value_from()->set_parameter("tag",_dialog_state._local_tag);

    notify->set_content(new Presence_Message_Content(get_dialog_config()->_sip_identity->get_sip_uri().get_string(),
                                                     toId->get_sip_uri().get_string(),
                                                     _online_status,
                                                     _online_status
                                                     ));

    SRef<Sip_Message*> pktr(*notify);

    Sip_SMCommand scmd( pktr, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer );

    get_sip_stack()->enqueue_command(scmd, HIGH_PRIO_QUEUE );
}

bool Sip_Dialog_Presence_Server::handle_command(const Sip_SMCommand &c)
{
    my_err << "Sip_Dialog_Presence_Server returning dialogs handleCommand"<< endl;
    bool handled = Sip_Dialog::handle_command(c);

    if (!handled && c.get_type()==Sip_SMCommand::COMMAND_STRING && c.get_command_string().get_op()==Sip_Command_String::no_transactions)
    {
        return true;
    }
    return handled;
}
