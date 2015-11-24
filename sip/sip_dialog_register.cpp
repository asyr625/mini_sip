#include <iostream>

#include "sip_dialog_register.h"
#include "sip_command_string.h"
#include "sip_transition_utils.h"

#include "sip_header_contact.h"
#include "sip_header_expires.h"
#include "sip_header_snake_sm.h"
#include "sip_header_from.h"
#include "sip_header_supported.h"

#define TIME_REREGISTER_AHEAD_S 5

bool Sip_Dialog_Register::a0_start_trying_register( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::proxy_register, Sip_SMCommand::dialog_layer,
                Sip_SMCommand::dialog_layer))
    {

        //Set user and password for the register
        if (command.get_command_string().get_param()!="" && command.get_command_string().get_param2()!="")
        {
            SRef<Sip_Credential*> cred;
            cred = get_dialog_config()->_sip_identity->get_credential();

            cred->set( command.get_command_string().get_param(),
                   command.get_command_string().get_param2() );
        }

        //Set expires param ... in seconds (in param3)
        if (command.get_command_string().get_param3()!="" )
        {
            get_dialog_config()->_sip_identity->get_sip_registrar()->set_register_expires( command.get_command_string().get_param3() );
        }

        //if it comes with an identity ... use it to filter out commands not for this dialog ...
        if (command.get_command_string()["identityId"]!="" )
        {
            std::string identity;
            identity = command.get_command_string()["identityId"];
            if( identity != get_dialog_config()->_sip_identity->get_id() )
            {
                //we got a proxy_register not for our identity ...
                return false;
            }
        }

        ++_dialog_state._seq_no;
        send_register();

        Command_String cmdstr( _dialog_state._call_id, Sip_Command_String::register_sent);
        cmdstr["identityId"] = get_dialog_config()->_sip_identity->get_id();
        get_sip_stack()->get_callback()->handle_command("gui", cmdstr );

        return true;
    }
    return false;
}

bool Sip_Dialog_Register::a1_trying_registred_2xx( const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::transaction_layer,
                Sip_SMCommand::dialog_layer, "2**"))
    {

///FIXME: XXXXXX Removed socket from packet - this functionality needs to be looked over
///		regcall->getDialogContainer()->getPhoneConfig()->proxyConnection = command.get_command_packet()->getSocket();

        //Mark the identity as currently registered (or not, maybe we are unregistering)

        SRef<Sip_Message*> pkt = command.get_command_packet();
        SRef<Sip_Header_Value_Contact *> c = pkt->get_header_value_contact();
        SRef<Sip_Header_Value_Expires *> expiresHeader = pkt->get_header_value_expires();
        std::list<Sip_Uri> contacts;

        // TODO add all contacts from the response
        if( c ){
            contacts.push_back( c->get_uri() );
        }
        get_dialog_config()->_sip_identity->set_registered_contacts ( contacts );
        int expires = 3600;
        if(expiresHeader)
          expires = expiresHeader->get_timeout();
        if(c)
            expires = c->get_expires();
        get_dialog_config()->_sip_identity->set_is_registered ( expires > 0 );

        Command_String cmdstr( _dialog_state._call_id, Sip_Command_String::register_ok,
            get_dialog_config()->_sip_identity->get_sip_registrar()->get_uri().get_ip());
        cmdstr["identityId"] = get_dialog_config()->_sip_identity->get_id();
        if (get_gui_feedback())
        {
            get_sip_stack()->get_callback()->handle_command("gui", cmdstr );
            set_gui_feedback(false);
        }

        //this is for the shutdown dialog
        Sip_SMCommand cmd( cmdstr, Sip_SMCommand::dialog_layer, Sip_SMCommand::dispatcher );
        get_sip_stack()->enqueue_command( cmd, HIGH_PRIO_QUEUE );

        SRef<Sip_Header_Value*> snakehdr = pkt->get_header_value_no(SIP_HEADER_TYPE_SNAKESM, 0);
        if (snakehdr)
        {
            SRef<Sip_Header_Value_Snake_SM*> h = dynamic_cast<Sip_Header_Value_Snake_SM*>(*snakehdr);
            std::string serviceManager=h->get_string();
            Command_String cmdstr(_dialog_state._call_id, expires > 0 ? "services_manager_connect" : "services_manager_disconnect", serviceManager);
            try{
                get_sip_stack()->get_callback()->handle_command("snake", cmdstr );
            }catch(Subsystem_Not_Found_Exception&)
            {
                my_err <<"WARNING: Can not set snake service manager: subsystem <snake> not found."<<std::endl;
            }
        }
        //request a timeout to retx a proxy_register only if we are registered ...
        //otherwise we would just be unregistering every now and then ...
        if( get_dialog_config()->_sip_identity->is_registered () )
        {
            int tsec_s = std::max(get_dialog_config()->_sip_identity->get_sip_registrar()->get_register_expires_int()-TIME_REREGISTER_AHEAD_S, 5); //hard-code a limit to faster than 5s re-register

            request_timeout( tsec_s * 1000, Sip_Command_String::proxy_register);
        }

        //Un-registering is done in the same dialog. With (a
        //configuration of?) opensips, we don't un-register
        //correctly by using the authentication state from
        //when we registred.
//		clearAuthentications();

        SRef<Sip_Response*> resp( (Sip_Response *)*command.get_command_packet());
        update_authentications( *resp );
        return true;
    }
    return false;
}

bool Sip_Dialog_Register::a2_trying_trying_40x( const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::transaction_layer,
                         Sip_SMCommand::dialog_layer, "401\n407"))
    {
        //extract authentication info from received response
        SRef<Sip_Response*> resp( (Sip_Response *)*command.get_command_packet());

        if( !update_authentications( *resp ) )
        {
            // Fall through to a3
            return false;
        }

        ++_dialog_state._seq_no;
        send_register();
        //TODO: inform GUI
        return true;
    }
    else if(transition_match(Sip_Response::type, command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer, "403"))
    {
        Command_String cmdstr( _dialog_state._call_id, Sip_Command_String::register_failed_authentication,
            get_dialog_config()->_sip_identity->get_sip_registrar()->get_uri().get_ip());
        cmdstr["identityId"] = get_dialog_config()->_sip_identity->get_id();
        if (get_gui_feedback())
        {
            get_sip_stack()->get_callback()->handle_command("gui", cmdstr );
            set_gui_feedback(false);
        }

        //this is for the shutdown dialog
        Sip_SMCommand cmd( cmdstr, Sip_SMCommand::dialog_layer, Sip_SMCommand::dispatcher );
        get_sip_stack()->enqueue_command( cmd, HIGH_PRIO_QUEUE );

        return true;
    }
    else if(transition_match(Sip_Response::type, command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer, "404"))
    {
        Command_String cmdstr( _dialog_state._call_id, Sip_Command_String::register_failed,
            get_dialog_config()->_sip_identity->get_sip_registrar()->get_uri().get_ip());
        cmdstr["identityId"] = get_dialog_config()->_sip_identity->get_id();
        if (get_gui_feedback())
        {
            get_sip_stack()->get_callback()->handle_command("gui", cmdstr );
            set_gui_feedback(false);
        }

        //this is for the shutdown dialog
        Sip_SMCommand cmd( cmdstr, Sip_SMCommand::dialog_layer, Sip_SMCommand::dispatcher );
        get_sip_stack()->enqueue_command( cmd, HIGH_PRIO_QUEUE );

        return true;
    }
    return false;
}

bool Sip_Dialog_Register::a3_trying_askpassword_40x( const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer,
                "401\n407"))
    {
        std::string realm = find_unauthenticated_realm();

        //TODO: Ask password
        Command_String cmdstr( _dialog_state._call_id, Sip_Command_String::ask_password,
            get_dialog_config()->_sip_identity->get_sip_registrar()->get_uri().get_ip());
        cmdstr["identityId"] = get_dialog_config()->_sip_identity->get_id();
        cmdstr["identityUri"] = get_dialog_config()->_sip_identity->get_sip_uri().get_string();
        cmdstr["realm"] = realm;

        get_sip_stack()->get_callback()->handle_command("gui", cmdstr );
        //authentication info from received response is extracted in a2
        return true;
    }
    return false;
}

bool Sip_Dialog_Register::a5_askpassword_trying_setpassword( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::setpassword, Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer))
    {
        std::string realm = command.get_command_string()["realm"];

            //We store the new credentials for this dialogs
            //configuration. Note that it is not saved for the
            //next time minisip is started.
        const std::string &user = command.get_command_string().get_param();
        const std::string &pass = command.get_command_string().get_param2() ;

        SRef<Sip_Credential*> cred =
            new Sip_Credential( user, pass, realm );

        add_credential( cred );

        ++_dialog_state._seq_no;

        send_register();
        return true;
    }
    return false;
}

//bool Sip_Dialog_Register::a6_askpassword_registred_2xx( const Sip_SMCommand &command);
bool Sip_Dialog_Register::a9_askpassword_failed_cancel( const Sip_SMCommand &command)
{
    if (transition_match(command, "cancel_register", Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer))
    {
        //Mark the identity as currently un-registered
        get_dialog_config()->_sip_identity->set_is_registered ( false );

#ifdef DEBUG_OUTPUT
        my_dbg("signaling/sip") << "WARNING: Sip_DialogRegister::a9: unimplemented section reached"<<std::endl;
#endif
        return true;
    }
    return false;
}

bool Sip_Dialog_Register::a10_trying_failed_transporterror( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::transport_error, IGN, Sip_SMCommand::dialog_layer))
    {
        //Mark the identity as currently un-registered
        get_dialog_config()->_sip_identity->set_is_registered ( false );

        Command_String cmdstr( _dialog_state._call_id, Sip_Command_String::transport_error);
        cmdstr["identityId"] = get_dialog_config()->_sip_identity->get_id();
        cmdstr["identityUri"] = get_dialog_config()->_sip_identity->get_sip_uri().get_string();
        get_sip_stack()->get_callback()->handle_command("gui", cmdstr );
        return true;
    }
    return false;
}

bool Sip_Dialog_Register::a12_registred_trying_proxyregister( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::proxy_register, Sip_SMCommand::dialog_layer,
                Sip_SMCommand::dialog_layer))
    {

        cancel_timeout(Sip_Command_String::proxy_register);

        //Set proxy username and password
        if (command.get_command_string().get_param()!="" && command.get_command_string().get_param2()!="")
        {
            SRef<Sip_Credential*> cred;
            cred = get_dialog_config()->_sip_identity->get_credential();

            cred->set( command.get_command_string().get_param(),
                   command.get_command_string().get_param2() );
        }

        //Set expires param ... in seconds
        if (command.get_command_string().get_param3()!="" )
        {
            get_dialog_config()->_sip_identity->get_sip_registrar()->set_register_expires( command.get_command_string().get_param3() );
        }

        //if it comes with an identity ... use it to filter out commands not for this dialog ...
        if (command.get_command_string()["identityId"]!="" )
        {
            std::string identity;
            identity = command.get_command_string()["identityId"];
            if( identity != get_dialog_config()->_sip_identity->get_id() )
            {
                //we got a proxy_register not for our identity ...
                return false;
            }
        }

        ++_dialog_state._seq_no;
        send_register();

        Command_String cmdstr(_dialog_state._call_id, Sip_Command_String::register_sent);
        cmdstr["identityId"] = get_dialog_config()->_sip_identity->get_id();
        cmdstr["identityUri"] = get_dialog_config()->_sip_identity->get_sip_uri().get_string();
        get_sip_stack()->get_callback()->handle_command("gui", cmdstr );
        return true;
    }
    return false;
}

bool Sip_Dialog_Register::a13_failed_terminated_notransactions( const Sip_SMCommand &command)
{
    if( transition_match(command, Sip_Command_String::no_transactions, Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer) )
    {

        _dialog_state._is_terminated=true;
        Command_String cmdstr ( _dialog_state._call_id,
                Sip_Command_String::call_terminated);
        cmdstr["identityId"] = get_dialog_config()->_sip_identity->get_id();

        Sip_SMCommand cmd(cmdstr, Sip_SMCommand::dialog_layer, Sip_SMCommand::dispatcher);

        get_sip_stack()->enqueue_command( cmd, HIGH_PRIO_QUEUE );

        return true;
    }
    return false;
}

bool Sip_Dialog_Register::a14_trying_trying_1xx( const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer, "1**"))
    {
        return true;
    }
    return false;
}

bool Sip_Dialog_Register::a15_service_unavailable( const Sip_SMCommand &command)
{
    if(transition_match(Sip_Response::type, command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer, "503"))
    {
        Command_String cmdstr( _dialog_state._call_id, Sip_Command_String::register_failed,
                    get_dialog_config()->_sip_identity->get_sip_registrar()->get_uri().get_ip());
        cmdstr["identityId"] = get_dialog_config()->_sip_identity->get_id();
        if (get_gui_feedback())
        {
            get_sip_stack()->get_callback()->handle_command("gui", cmdstr );
            set_gui_feedback(false);
        }

        //this is for the shutdown dialog
        Sip_SMCommand cmd( cmdstr, Sip_SMCommand::dialog_layer, Sip_SMCommand::dispatcher );
        get_sip_stack()->enqueue_command( cmd, HIGH_PRIO_QUEUE );

        return true;
    }
    return false;
}



void Sip_Dialog_Register::set_up_state_machine()
{
    State<Sip_SMCommand, std::string> *s0_start =
        new State<Sip_SMCommand,std::string>(this,"s0_start");
    add_state(s0_start);

    State<Sip_SMCommand, std::string> *s1_trying =
        new State<Sip_SMCommand,std::string>(this,"s1_trying");
    add_state(s1_trying);

    State<Sip_SMCommand, std::string> *s2_registred =
        new State<Sip_SMCommand,std::string>(this,"s2_registred");
    add_state(s2_registred);

    State<Sip_SMCommand, std::string> *s4_askpassword=
        new State<Sip_SMCommand,std::string>(this,"s4_askpassword");
    add_state(s4_askpassword);

    State<Sip_SMCommand, std::string> *s5_failed=
        new State<Sip_SMCommand,std::string>(this,"s5_failed");
    add_state(s5_failed);

    State<Sip_SMCommand, std::string> *terminated=
        new State<Sip_SMCommand,std::string>(this,"terminated");
    add_state(terminated);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_start_trying_register",
        (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Register::a0_start_trying_register,
        s0_start, s1_trying);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_trying_registred_2xx",
        (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Register::a1_trying_registred_2xx,
        s1_trying, s2_registred);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_trying_trying_40x",
        (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Register::a2_trying_trying_40x,
        s1_trying, s1_trying);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_trying_askpassword_40x",
        (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Register::a3_trying_askpassword_40x,
        s1_trying, s4_askpassword);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_askpassword_askpassword_setpass",
        (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Register::a5_askpassword_trying_setpassword,
        s4_askpassword, s1_trying);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_askpassword_failed_cancel",
        (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Register::a9_askpassword_failed_cancel,
        s4_askpassword, s5_failed);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_trying_failed_transporterror",
        (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Register::a10_trying_failed_transporterror,
        s1_trying, s5_failed);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_registred_trying_proxyregister",
        (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Register::a12_registred_trying_proxyregister,
        s2_registred, s1_trying);

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_failed_terminated_notransactions",
        (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Register::a13_failed_terminated_notransactions,
        s5_failed, terminated );

    new State_Transition<Sip_SMCommand,std::string>(this, "transition_trying_trying",
        (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Register::a14_trying_trying_1xx,
        s1_trying, s1_trying );
    new State_Transition<Sip_SMCommand,std::string>(this, "transition_trying_failed_service_unavailable",
        (bool (State_Machine<Sip_SMCommand,std::string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Register::a15_service_unavailable,
        s1_trying, s5_failed);

    set_current_state(s0_start);
}

Sip_Dialog_Register::Sip_Dialog_Register(SRef<Sip_Stack*> stack, SRef<Sip_Identity*> identity)
    : Sip_Dialog(stack, identity, ""),
      _fail_count(0),
      _gui_feedback(true)
{
    set_up_state_machine();
    _my_domain = get_dialog_config()->_sip_identity->get_sip_uri().get_ip();
}

Sip_Dialog_Register::~Sip_Dialog_Register()
{
}

void Sip_Dialog_Register::update_fail_count()
{
    _fail_count ++;
}

uint32_t Sip_Dialog_Register::get_fail_count()
{
    return _fail_count;
}

bool Sip_Dialog_Register::get_gui_feedback()
{
    return _gui_feedback;
}

void Sip_Dialog_Register::set_gui_feedback(bool fb)
{
    _gui_feedback = fb;
}

bool Sip_Dialog_Register::handle_command(const Sip_SMCommand &command)
{
    if (command.get_type()==Sip_SMCommand::COMMAND_PACKET
            && !(command.get_destination()==Sip_SMCommand::dialog_layer
                 /*|| command.get_destination()==Sip_SMCommand::ANY*/))
    {
        my_err << "WARNING: UNEXPECTED: received packet in Sip_DialogRegister: "<<command.get_command_packet()->get_description() << std::endl;
        return false;
    }

    if (command.get_type()==Sip_SMCommand::COMMAND_STRING
            && (command.get_destination()==Sip_SMCommand::dialog_layer /*|| command.get_destination()==Sip_SMCommand::ANY*/)
            && (command.get_command_string().get_op()==Sip_Command_String::proxy_register)
            && (command.get_command_string()["identityId"] == get_dialog_config()->_sip_identity->get_id()
                || (command.get_command_string()["identityId"] == ""
                    && (command.get_command_string()["proxy_domain"]==""
                        || command.get_command_string()["proxy_domain"]== get_dialog_config()->_sip_identity->get_sip_uri().get_ip()) )))
    {
        return Sip_Dialog::handle_command(command);
    }

    if (command.get_type()==Sip_SMCommand::COMMAND_STRING
            && (command.get_destination()==Sip_SMCommand::dialog_layer
                /*|| command.get_destination()==Sip_SMCommand::ANY*/))
    {

        if (_dialog_state._call_id == command.get_command_string().get_destination_id())
        {
            //getPhone()->log(LOG_INFO, "Sip_DialogRegister::handle_command: found matching call id");
        }else{
            //getPhone()->log(LOG_INFO, "Sip_DialogRegister::handle_command: not matching call id for this call");
            return false;
        }
    }

    bool ret = Sip_Dialog::handle_command(command);
    if (!ret && command.get_type()==Sip_SMCommand::COMMAND_STRING
            && command.get_command_string().get_op()==Sip_Command_String::no_transactions)
    {
        return true;
    }
    return ret;
}

void Sip_Dialog_Register::send_register()
{
    SRef<Sip_Identity*> identity = get_dialog_config()->_sip_identity;

    const Sip_Uri &contact = get_dialog_config()->get_contact_uri(true); //if udp, use stun
    int expires = identity->get_sip_registrar()->get_register_expires_int();

    SRef<Sip_Header_Value_Contact *> contactHdr =
            new Sip_Header_Value_Contact(contact, expires);
    const std::string &instanceId = get_sip_stack()->get_stack_config()->instance_id;

    if( !instanceId.empty() ){
        contactHdr->set_parameter("+sip.instance", instanceId);
        contactHdr->set_parameter("reg-id", identity->get_id());
    }

    SRef<Sip_Request*> reg= Sip_Request::create_sip_message_register(
                _dialog_state._call_id,
                identity->get_sip_registrar()->get_uri(),
                identity->get_sip_uri(),
                contactHdr,
                _dialog_state._seq_no
                );

    reg->get_header_value_from()->set_parameter( "tag", _dialog_state._local_tag );

    add_authorizations( reg );
    add_route( reg );

    if( !instanceId.empty() ){
        // Draft-Outbound needs path support, and
        // Draft-GRUU needs gruu
        reg->add_header(new Sip_Header(new Sip_Header_Value_Supported("path, gruu")));
    }

    Sip_SMCommand cmd(*reg, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE);
}
