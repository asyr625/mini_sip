#include "default_dialog_handler.h"

#include "network_exception.h"
#include "dns_naptr_query.h"

#include "sip_dialog_register.h"
#include "sip_header_from.h"
#include "sip_header_to.h"
#include "sip_header_accept_contact.h"
#include "sip_header_allow.h"

#include "sip_message.h"
#include "sip_message_content_im.h"
#include "sip_request.h"
#include "sip_command_string.h"
#include "my_assert.h"
#include "timestamp.h"

#include "sip_dialog_voip_client.h"
#include "sip_dialog_voip_server.h"
#include "sip_dialog_presence_client.h"
#include "sip_dialog_presence_server.h"


#ifdef _WIN32_WCE
#	include "minisip_wce_extra_includes.h"
#endif

#include "subsystem_media.h"
#include "dbg.h"

using namespace std;

Default_Dialog_Handler::Default_Dialog_Handler(SRef<Sip_Stack*> stack, SRef<Sip_Configuration*> pconf,
                                               SRef<Subsystem_Media*> subsystemMedia)
    : _sip_stack(stack),
      _conf(pconf),
      _subsystem_media(subsystemMedia),
      _outside_dialog_seq_no(1)
{
}

Default_Dialog_Handler::~Default_Dialog_Handler()
{
}

std::string Default_Dialog_Handler::get_name()
{
    return "DefaultDialogHandler";
}
SRef<Sip_Identity *> Default_Dialog_Handler::lookup_target(const Sip_Uri &uri)
{
    SRef<Sip_Identity *> id = NULL;

    id = _conf->get_identity( uri );

    if (!id){
        my_err <<"WARNING: Could not find local identity - using default"<<endl;
        id = _conf->_default_identity;
    }

    return id;
}

bool Default_Dialog_Handler::handle_command_packet(SRef<Sip_Message*> pkt )
{
    if (pkt->get_type()=="INVITE"){
        SRef<Sip_Request*> inv = dynamic_cast<Sip_Request*>(*pkt);

        SRef<Sip_Identity *> id = lookup_target(inv->get_uri());

        // get a session from the mediaHandler
        SRef<Session *> mediaSession =
                _subsystem_media->create_session( id, pkt->get_call_id() );

        SRef<Sip_Dialog*> voipCall;
        voipCall = new Sip_Dialog_Voip_Server(_sip_stack,
                                             id,
                                             _conf,
                                             mediaSession,
                                             pkt->get_call_id());
        _sip_stack->add_dialog(voipCall);


        Sip_SMCommand cmd(pkt, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer);
        _sip_stack->enqueue_command(cmd, HIGH_PRIO_QUEUE );
        my_dbg("signaling/sip") << cmd << endl;
        return true;
    }


    if (pkt->get_type()=="MESSAGE")
    {

        SRef<Sip_Request*> im = (Sip_Request*)*pkt;

#ifdef DEBUG_OUTPUT
        my_dbg("signaling/sip") << "DefaultDialogHandler:: creating new server transaction for incoming SipIMMessage" << endl;
#endif
        send_imOk( im );

        my_assert(dynamic_cast<Sip_Message_Content_IM*>(*im->get_content())!=NULL);

        SRef<Sip_Message_Content_IM*> imref = (Sip_Message_Content_IM*)*im->get_content();

        string from =  im->get_header_value_from()->get_uri().get_user_name()+"@"+
                im->get_header_value_from()->get_uri().get_ip();
        string to =  im->get_header_value_to()->get_uri().get_user_name()+"@"+
                im->get_header_value_to()->get_uri().get_ip();

        Command_String cmdstr("", Sip_Command_String::incoming_im, imref->get_string(), from, to );
        _sip_stack->get_callback()->handle_command("gui", cmdstr );
        return true;
    }

    // Reject unimplemented or unhandled request methods
    if( pkt->get_type()!=Sip_Response::type )
    {
        int statusCode;
        const char *reasonPhrase;

        if (pkt->get_type()=="BYE" ||
                pkt->get_type()=="CANCEL" ||
                pkt->get_type()=="ACK")
        {
            statusCode = 481;
            reasonPhrase = "Call/transaction does not exist";
        }else {
            statusCode = 405;
            reasonPhrase = "Method Not Allowed";
        }

        SRef<Sip_Request*> req = (Sip_Request*)*pkt;
        SRef<Sip_Response*> resp = new Sip_Response( statusCode, reasonPhrase, req );

        if (statusCode==405)
            resp->add_header(new Sip_Header(new Sip_Header_Value_Allow("INVITE,MESSAGE,BYE,ACK,OPTIONS,PRACK") ));

        Sip_SMCommand cmd( *resp, Sip_SMCommand::dialog_layer,
                           Sip_SMCommand::transaction_layer );
        // Send responses directly to the transport layer bypassing
        // the transaction layer
        _sip_stack->enqueue_command(cmd, HIGH_PRIO_QUEUE);
        return true;
    }

    my_dbg("signaling/sip") << "DefaultDialogHandler ignoring " << pkt->get_string() << endl;

    return false;
}

bool Default_Dialog_Handler::handle_command_string(Command_String &cmdstr )
{
    if (cmdstr.get_op() == Sip_Command_String::start_presence_client)
    {
        cerr << "DefaultDialogHandler: Creating Sip_Dialog_Presence_Client for start_presence_client command"<< endl;

        SRef<Sip_Dialog_Presence_Client*> pres(new Sip_Dialog_Presence_Client(_sip_stack, _conf->_default_identity, _conf->useSTUN ));

        _sip_stack->add_dialog( SRef<Sip_Dialog*>(*pres) );

        Command_String command(cmdstr);
        cmdstr.set_destination_id(pres->get_call_id());
        Sip_SMCommand cmd( cmdstr, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer);
        _sip_stack->enqueue_command(cmd, HIGH_PRIO_QUEUE );

        return true;
    }

    if (cmdstr.get_op() == Sip_Command_String::start_presence_server)
    {
        cerr << "DefaultDialogHandler: Creating Sip_Dialog_Presence_Server for start_presence_server command"<< endl;

        SRef<Sip_Dialog_Presence_Server*> pres(new Sip_Dialog_Presence_Server(_sip_stack, _conf->_default_identity, _conf->useSTUN ));

        _sip_stack->add_dialog( SRef<Sip_Dialog*>(*pres) );

        Command_String command(cmdstr);
        cmdstr.set_destination_id(pres->get_call_id());
        Sip_SMCommand cmd( cmdstr, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer);
        _sip_stack->enqueue_command(cmd, HIGH_PRIO_QUEUE );

        return true;
    }

    if (cmdstr.get_op() == Sip_Command_String::outgoing_im)
    {
        ++_outside_dialog_seq_no;
        send_im( cmdstr.get_param(), _outside_dialog_seq_no, cmdstr.get_param2() );
        return true;
    }

    if (cmdstr.get_op() == Sip_Command_String::proxy_register)
    {
        SRef<Sip_Identity *> identity;
        identity = _conf->get_identity( cmdstr["identityId"] );
        if (!identity)
        {
            my_dbg("signaling/sip")<< "WARNING: unknown identity"<<endl;
            return true;
        }

        string proxyDomainArg = cmdstr["proxy_domain"];

        /* Use appropriate identity ...
        */
        if( ! identity.is_null() ) {
            ;
        } else if (_conf->_pstn_identity && (cmdstr.get_destination_id()=="pstn"
                                           || (proxyDomainArg!="" && proxyDomainArg==_conf->_pstn_identity->get_sip_uri().get_ip()))){
            identity=_conf->_pstn_identity;
        }

        SRef<Sip_Dialog_Register*> reg(new Sip_Dialog_Register(_sip_stack, identity));

        _sip_stack->add_dialog( SRef<Sip_Dialog*>(*reg) );
        cmdstr.set_destination_id( reg->get_call_id() );
        Sip_SMCommand cmd( cmdstr, Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer);
        _sip_stack->enqueue_command(cmd, HIGH_PRIO_QUEUE);
        return true;
    }

    my_dbg("signaling/sip") << "DefaultDialogHandler ignoring command " << cmdstr.get_string() << endl;

    return false;
}

bool Default_Dialog_Handler::handle_command(const Sip_SMCommand &command)
{
    my_dbg("signaling/sip") << "DefaultDialogHandler: got command "<< command << endl;
    int dst = command.get_destination();
    if ( dst!=Sip_SMCommand::dialog_layer)
        return false;

    if (command.get_type()==Sip_SMCommand::COMMAND_PACKET)
    {
        return handle_command_packet( command.get_command_packet() );
    }
    else
    {
        my_assert(command.get_type()==Sip_SMCommand::COMMAND_STRING);
        Command_String cmdstr = command.get_command_string();
        return handle_command_String( cmdstr/*, dispatchCount */);
    }
}

void Default_Dialog_Handler::handle_command(std::string subsystem, const Command_String &cmd)
{
    assert(subsystem=="sip");
    my_err << "DefaultDialogHandler::handle_command(subsystem,cmd): Can not handle: "<< cmd.get_string() << endl;
}

Command_String Default_Dialog_Handler::handle_command_resp(std::string subsystem, const Command_String &cmd)
{
    assert(subsystem=="sip");
    assert(cmd.get_op()=="invite");//TODO: no assert, return error message instead

    string user = cmd.get_param();
    bool gotAtSign;
#ifdef ENABLE_TS
    ts.save( INVITE_START );
#endif
    int startAddr=0;

    if (user[0]=='<' && user[user.length()-1]=='>')
        user = user.substr(1,user.length()-2);


    if (user.substr(0,4)=="sip:")
        startAddr = 4;
    else if (user.substr(0,5)=="sips:")
        startAddr = 5;
    else if( user.substr(0, 4) == "isn:")
    {
        SRef<Dns_Naptr_Query*> query = Dns_Naptr_Query::create();
        if( query->resolve_isn( user.substr( 4 )))
            user = query->get_result();
    }
    else if( user.substr(0, 5) == "enum:" )
    {
        SRef<Dns_Naptr_Query*> query = Dns_Naptr_Query::create();
        if( query->resolve_enum( user.substr( 5 )))
            user = query->get_result();
    }

    bool onlydigits = true;
    SRef<Sip_Identity *> id;

    for (unsigned i=startAddr; i<user.length(); i++)
        if (user[i]<'0' || user[i]>'9')
            onlydigits = false;

    id = ( onlydigits && _conf->_use_pstn_proxy )?
                _conf->_pstn_identity:
                _conf->_default_identity;

    if( !id )
    {
        my_err << "ERROR: could not determine what local identity to use" << endl;
        Command_String err("","error", "No matching local identity");
        return err;
    }

    gotAtSign = ( user.find("@", startAddr) != string::npos );

    if( !gotAtSign && id )
    {
        id->lock();
        user += "@" + id->get_sip_uri().get_ip();
        id->unlock();
    }
    SRef<Sip_Dialog_Voip*> voipCall = new Sip_Dialog_Voip_Client(_sip_stack, id, _conf->_use_stun, _conf->_use_anat, NULL);;

    SRef<Session *> mediaSession = _subsystem_media->create_session( id, voipCall->get_call_id() );
    voipCall->set_media_session( mediaSession );

    _sip_stack->add_dialog(*voipCall);

    /* creating an invite command string */

    string sdpBody = cmd.get_param3();

    Command_String inv(voipCall->get_call_id(), Sip_Command_String::invite, user ,cmd.get_param2(), cmd.get_param3());


#ifdef ENABLE_TS
    ts.save( TMP );
#endif

    Sip_SMCommand c(Sip_SMCommand(inv, Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer)); //TODO: send directly to dialog instead

    _sip_stack->handle_command(c);

    mediaSession->set_call_id( voipCall->get_call_id() );

    string cid = voipCall->get_call_id();

    Command_String ret(cid,"invite_started");
    return ret;
}

void Default_Dialog_Handler::send_im_ok(SRef<Sip_Request*> bye)
{
    SRef<Sip_Response*> ok= new Sip_Response( 200,"OK", bye );
    ok->get_header_value_to()->set_parameter("tag","libminisip");

    SRef<Sip_Message*> pref(*ok);
    Sip_SMCommand cmd( pref, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    _sip_stack->enqueue_command(cmd, HIGH_PRIO_QUEUE);
}

void Default_Dialog_Handler::send_im(std::string msg, int im_seq_no, std::string toUri)
{
    size_t posAt;

    posAt = toUri.find("@");
    if( posAt == string::npos ) { //toUri does not have a domain ...
        //get one, from the default identity
        if( _conf->_default_identity->get_sip_uri().get_ip() != "" )
        {
            toUri += "@" + _conf->_default_identity->get_sip_uri().get_ip();
        } else
        {
#ifdef DEBUG_OUTPUT
            cerr << "DefaultDialogHandler::send_im - toUri without domain" << endl;
#endif
        }
    }
#ifdef DEBUG_OUTPUT
    cerr << "DefaultDialogHandler::send_im - toUri = " << toUri <<  endl;
#endif

    SRef<Sip_Request*> im = Sip_Request::create_sip_message_im_message(
                itoa(rand()),	//Generate random callId
                toUri,
                _conf->_default_identity->get_sip_uri(),
                im_seq_no,
                msg
                );

    //Add outbount proxy route
    const list<Sip_Uri> &routes = _conf->_default_identity->get_route_set();
    im->add_routes( routes );

    //FIXME: there should be a SipIMDialog, just like for register messages ...
    // 	otherwise, we cannot keep track of local/remote tags, callids, etc ...
    //	very useful for matching incoming and outgoing IMs ...
    im->get_header_value_from()->set_parameter("tag","libminisip"); //we need a from tag ... anything ...

    SRef<Sip_Message*> pref(*im);
    Sip_SMCommand cmd( pref, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    _sip_stack->enqueue_command(cmd, HIGH_PRIO_QUEUE);
}



