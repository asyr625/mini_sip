#include "sip_dialog_voip.h"

#include "my_assert.h"
#include "sip_transition_utils.h"

#include "sip_header_warning.h"
#include "sip_header_contact.h"
#include "sip_header_from.h"
#include "sip_header_route.h"
#include "sip_header_require.h"
#include "sip_header_to.h"

#include "sip_message_content_mime.h"
#include "sip_message_content.h"
#include "sip_command_string.h"
#include "string_utils.h"
#include "base64.h"
#include "timestamp.h"
#include "termmanip.h"
#include "dbg.h"
#include "sip_smcommand.h"

#include <time.h>
#include "log_entry.h"

#include "sip_dialog_voip_client.h"
#include "video_fast_update_message_content.h"

#include "sdp_headero.h"

#include <time.h>
#include <string>
#include <stdlib.h>

#include <algorithm>
#include <sstream>
#include <iostream>
using namespace std;

#ifdef _WIN32_WCE
#	include "minisip_wce_extra_includes.h"
#endif

#define SIP_MEDIA_USE_MEDIAHANDLER_API 1

static string getReferredUri(SRef<Sip_Request*> req)
{
    string referredUri;
    SRef<Sip_Header_Value*> hval;
    if (req)
        hval = req->get_header_value_no(SIP_HEADER_TYPE_REFERTO, 0);

    if (hval)
        referredUri = hval->get_string();
    else{
        cerr << "WARNING: Referred to uri not found!"<<endl;
    }
    return referredUri;
}

bool Sip_Dialog_Voip::a1011_incall_incall_REINVITE( const Sip_SMCommand &command)
{
    if( !transition_match("INVITE", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer) )
    {
        return false;
    }

    SRef<Sip_Request*> inv = (Sip_Request *)*command.get_command_packet();

    if( inv->requires("100rel") ){
        // TODO reject unsupported extension
        return false;
    }

    set_last_invite(inv);
    _dialog_state.update_state( inv );

    SRef < Sdp_Packet *> lastSdp_Packet = media_session->get_last_sdp_packet();

    // Build peer uri used for authentication from remote uri,
    // but containing user and host only.
    Sip_Uri peer(_dialog_state._remote_uri);
    string peerUri = peer.get_protocol_id() + ":" + peer.get_user_ip_string();

    SRef<Sip_Response*> ok= new Sip_Response(200,"OK", get_last_invite() );
    ok->get_header_value_to()->set_parameter("tag",_dialog_state._local_tag);

    SRef<Sip_Header_Value *> contact =
            new Sip_Header_Value_Contact(
                get_dialog_config()->get_contact_uri(use_stun),
                -1); //set expires to -1, we do not use it (only in register)
    ok->add_header( new Sip_Header(*contact) );


    //	if ( getChild() != true ) {

    if(!sort_mime(*inv->get_content(), peerUri, 10))
    {
        my_err << "No MIME match" << std::endl;
        return false;
    }


    /* un mute all the streamers and the new ones */
    Command_String cmda( _dialog_state._call_id,
                        "set_session_sound_settings",
                        "senders", "ON" );
    get_sip_stack()->get_callback()->handle_command("media", cmda );

    SRef<Sdp_Packet *> sdp;
    if (media_session){
#ifdef ENABLE_TS
        ts.save("getSdpAnswer");
#endif
        sdp = media_session->get_sdp_answer();
#ifdef ENABLE_TS
        ts.save("getSdpAnswer");
#endif

        if( !sdp ){
            // FIXME: this most probably means that the
            // creation of the MIKEY message failed, it
            // should not happen
            return false;
        }
    }
    get_media_session()->refresh(); /* every not started receiver and sender starts*/
    if ( sdp->get_string().compare(lastSdp_Packet->get_string() ) != 0 )
    {
        SRef <Sdp_HeaderO* > headerO = dynamic_cast<Sdp_HeaderO * > (*(sdp->get_headers()[1]));
        int newVersionNumber  = atoi(headerO->get_version().c_str() );
        newVersionNumber++;
        std::stringstream out;
        out << newVersionNumber;
        headerO->set_version(  out.str() );
    }

    ok->set_content( *sdp );
    /*	}// getChild finish
       else  {
           ok->set_content(*lastSdp_Packet);
       }
   */
    SRef<Sip_Message*> pref(*ok);
    Sip_SMCommand cmd( pref, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE );

    return true;
}

bool Sip_Dialog_Voip::a1012_incall_incall_INFO(const Sip_SMCommand &command)
{
    if(!transition_match("INFO", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer))
        return false;
    Sip_Request *request = dynamic_cast<Sip_Request *>(*command.get_command_packet());
    if(request == NULL)
        return false;
    SRef<Sip_Message_Content *> content = request->get_content();
    string contentStr = content ? content->get_string() : "";
    transform(contentStr.begin(), contentStr.end(), contentStr.begin(), ::tolower);
    if(contentStr.find("media_control") != string::npos
            && contentStr.find("vc_primitive") != string::npos
            && contentStr.find("to_encoder") != string::npos
            && contentStr.find("picture_fast_update") != string::npos)
    {
        send_info_ok(request);
        if(media_session)
            media_session->video_keyframe_request_arrived();
        else
            cout << "Sip_DialogVoip::a1012_incall_incall_INFO() warning, video keyframe request could not be handled" << endl;
        return true;
    }
    SRef<Sip_Response*> error= new Sip_Response(415, "Unsupported Media Type", request);
    error->get_header_value_to()->set_parameter("tag", _dialog_state._local_tag);
    SRef<Sip_Message*> pref(*error);
    Sip_SMCommand cmd(pref, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE );
    return true;
}

bool Sip_Dialog_Voip::a1019_ReInviteSent_Incall_200OK ( const Sip_SMCommand &command)
{

}

bool Sip_Dialog_Voip::a1001_incall_termwait_BYE( const Sip_SMCommand &command)
{
    if (transition_match("BYE", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer) &&
            _dialog_state._remote_tag != "")
    {
        SRef<Sip_Request*> bye = (Sip_Request*) *command.get_command_packet();
        if( get_log_entry() )
        {
            ((Log_Entry_Success *)(*( get_log_entry() )))->duration =
                    time( NULL ) - get_log_entry()->start;

            get_log_entry()->handle();
        }
        send_bye_ok(bye);
        Command_String cmdstr(_dialog_state._call_id, Sip_Command_String::remote_hang_up);
        get_sip_stack()->get_callback()->handle_command("gui", cmdstr);
        get_media_session()->stop();
        signal_if_no_transactions();
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip::a1002_incall_byerequest_hangup( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::hang_up, Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer))
    {
        ++_dialog_state._seq_no;
        send_bye(_dialog_state._seq_no);

        if (get_log_entry())
        {
            (dynamic_cast< Log_Entry_Success * >(*( get_log_entry() )))->duration = time( NULL ) - get_log_entry()->start;
            get_log_entry()->handle();
        }

        get_media_session()->stop();

        signal_if_no_transactions();
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip::a1003_byerequest_termwait_26( const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer,
                        "2**\n3**\n4**\n5**\n6**") )
    {
        /*		Sip_SMCommand cmd(
                Command_String( _dialog_state._call_id,
                    Sip_Command_String::call_terminated),
                    Sip_SMCommand::dialog_layer,
                    Sip_SMCommand::dispatcher);

        dispatcher->enqueue_command( cmd, HIGH_PRIO_QUEUE);
*/

        _dialog_state._is_terminated=true;

        /* Tell the GUI */
        Command_String cmdstr( _dialog_state._call_id, Sip_Command_String::call_terminated);
        get_sip_stack()->get_callback()->handle_command("gui", cmdstr );

        //this is for the shutdown dialog
        Command_String earlystr( _dialog_state._call_id, Sip_Command_String::call_terminated_early);
        Sip_SMCommand cmd( earlystr, Sip_SMCommand::dialog_layer, Sip_SMCommand::dispatcher);
        get_sip_stack()->enqueue_command( cmd, HIGH_PRIO_QUEUE );

        return true;
    }
    return false;
}

bool Sip_Dialog_Voip::a1101_termwait_terminated_notransactions( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::no_transactions, Sip_SMCommand::dialog_layer,
                        Sip_SMCommand::dialog_layer) )
    {
        last_invite=NULL;

        _dialog_state._is_terminated=true;
        Sip_SMCommand cmd(
                    Command_String( _dialog_state._call_id, Sip_Command_String::call_terminated),
                    Sip_SMCommand::dialog_layer,
                    Sip_SMCommand::dispatcher);

        get_sip_stack()->enqueue_command( cmd, HIGH_PRIO_QUEUE );
        /* Tell the GUI */
        //Command_String cmdstr( _dialog_state._call_id, Sip_Command_String::call_terminated);
        //sipStack->get_callback()->handle_command("gui", cmdstr );
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip::a1102_termwait_termwait_early( const Sip_SMCommand &command)
{
    if( transition_match(command, Sip_Command_String::hang_up, Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer) )
    {
        Command_String cmdstr( _dialog_state._call_id, Sip_Command_String::call_terminated_early);
        /* Tell the GUI, once only */
        if( notify_early_termination ) {
            get_sip_stack()->get_callback()->handle_command("gui", cmdstr );
            notify_early_termination = false;
        }
        Sip_SMCommand cmd( cmdstr, Sip_SMCommand::dialog_layer, Sip_SMCommand::dispatcher);
        get_sip_stack()->enqueue_command( cmd, HIGH_PRIO_QUEUE ); //this is for the shutdown dialog
        return true;
    }
    else if ( notify_early_termination && transition_match(Sip_Response::type, command, Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer, "2**"))
    {
        last_invite=NULL;
        //Notify the GUI and the dialog container ...
        Command_String cmdstr( _dialog_state._call_id, Sip_Command_String::call_terminated_early);

        /* Tell the GUI, once only */
        get_sip_stack()->get_callback()->handle_command("gui", cmdstr );
        notify_early_termination = false;

        //Tell the dialog container ...
        Sip_SMCommand cmd( cmdstr, Sip_SMCommand::dialog_layer, Sip_SMCommand::dispatcher);
        get_sip_stack()->enqueue_command( cmd, HIGH_PRIO_QUEUE ); //this is for the shutdown dialog
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip::a1201_incall_transferrequested_transfer( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::user_transfer, Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer))
    {
        string referredUri = command.get_command_string().get_param();
        ++_dialog_state._seq_no;
        send_refer(_dialog_state._seq_no, referredUri);

        return true;
    }
    return false;
}

bool Sip_Dialog_Voip::a1202_transferrequested_transferpending_202( const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer,
                        "202"))
    {
        SRef<Sip_Response*> resp( (Sip_Response*)*command.get_command_packet() );

        Command_String cmdstr(_dialog_state._call_id,
                             Sip_Command_String::transfer_pending
                             );
        get_sip_stack()->get_callback()->handle_command("gui", cmdstr );

        /* Minisip does not actually keep track of the transfer ...
        if the REFER is accepted by the far end, then shutdown the media
        */

        get_media_session()->stop();
        signal_if_no_transactions();
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip::a1203_transferrequested_incall_36( const Sip_SMCommand &command)
{
    if (transition_match(Sip_Response::type, command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer,
                        "3**\n4**\n5**\n6**"))
    {
        Command_String cmdstr( _dialog_state._call_id,
                              Sip_Command_String::transfer_refused,
                              command.get_command_packet()->get_warning_message());
        get_sip_stack()->get_callback()->handle_command("gui", cmdstr );
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip::a1204_transferpending_transferpending_notify( const Sip_SMCommand &command)
{
    bool ret = false;

    if (transition_match("NOTIFY", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer))
    {
        //this is just the same notify, lifted from the transaction up to the dialog ...
        //we can safely absorb it ...
        return true;
    }
    else if (transition_match("NOTIFY", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer))
    {
        SRef<Sip_Request*> notif;
        notif = (Sip_Request*)*command.get_command_packet();

        Sip_SMCommand cmd(command);
        cmd.set_destination(Sip_SMCommand::transaction_layer);
        cmd.set_source(command.get_source());
        get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE );

        send_notify_ok( notif );

        ret = true;
    }
    return ret;
}

bool Sip_Dialog_Voip::a1301_incall_transferaskuser_REFER( const Sip_SMCommand &command)
{
    if (transition_match("REFER", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer))
    {

        if (command.get_command_packet()->get_type()=="REFER"){
            lastRefer = dynamic_cast<Sip_Request*>(*command.get_command_packet());
        }else{
            lastRefer = NULL;
        }

        string referredUri = getReferredUri(lastRefer);

        /* Tell the GUI */
        Command_String cmdstr(_dialog_state._call_id,
                             Sip_Command_String::transfer_requested,
                             referredUri);
        get_sip_stack()->get_callback()->handle_command("gui", cmdstr );
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip::a1302_transferaskuser_transferstarted_accept( const Sip_SMCommand &command)
{
    if (transition_match(command,
                        Sip_Command_String::user_transfer_accept,
                        Sip_SMCommand::dialog_layer,
                        Sip_SMCommand::dialog_layer))
    {
        send_refer_ok();

        /* start a new call ... */
        string uri = getReferredUri(lastRefer);

        Command_String invite("",Sip_Command_String::invite, uri);
        Command_String resp = get_sip_stack()->handle_command_resp("sip",invite);
        string newCallId=resp.get_destination_id();

        /* Send the new _call_id to the GUI */
        Command_String cmdstr(_dialog_state._call_id, Sip_Command_String::call_transferred, newCallId);
        get_sip_stack()->get_callback()->handle_command("gui", cmdstr );

        return true;
    }
    return false;
}

bool Sip_Dialog_Voip::a1303_transferaskuser_incall_refuse( const Sip_SMCommand &command)
{
    if (transition_match(command,
                        Sip_Command_String::user_transfer_refuse,
                        Sip_SMCommand::dialog_layer,
                        Sip_SMCommand::dialog_layer))
    {
        send_refer_reject();
        return true;
    }
    return false;
}

void Sip_Dialog_Voip::set_up_state_machine()
{
    State<Sip_SMCommand,string> *s_incall=new State<Sip_SMCommand,string>(this,"incall");
    add_state(s_incall);

    State<Sip_SMCommand,string> *s_byerequest=new State<Sip_SMCommand,string>(this,"bye_request");
    add_state(s_byerequest);

    State<Sip_SMCommand,string> *s_termwait=new State<Sip_SMCommand,string>(this,"termwait");
    add_state(s_termwait);

    State<Sip_SMCommand,string> *s_terminated=new State<Sip_SMCommand,string>(this,"terminated");
    add_state(s_terminated);

    State<Sip_SMCommand,string> *s_endingchild=new State<Sip_SMCommand,string>(this,"endingchild");
    add_state(s_endingchild);


    // call transfer states
    State<Sip_SMCommand,string> *s_transferrequested=new State<Sip_SMCommand,string>(this,"transferrequested");
    add_state(s_transferrequested);

    State<Sip_SMCommand,string> *s_transferpending=new State<Sip_SMCommand,string>(this,"transferpending");
    add_state(s_transferpending);

    State<Sip_SMCommand,string> *s_transferaskuser=new State<Sip_SMCommand,string>(this,"transferaskuser");
    add_state(s_transferaskuser);

    State<Sip_SMCommand,string> *s_transferstarted=new State<Sip_SMCommand,string>(this,"transferstarted");
    add_state(s_transferstarted);

    // Re invite
    new State_Transition<Sip_SMCommand,string>(this, "transition_incall_incall_REINVITE",
                                              (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_DialogVoip::a1011_incall_incall_REINVITE,
                                              s_incall, s_incall);
    // INFO
    new State_Transition<Sip_SMCommand,string>(this, "transition_incall_incall_INFO",
                                              (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_DialogVoip::a1012_incall_incall_INFO,
                                              s_incall, s_incall);

    // Ending a call
    new State_Transition<Sip_SMCommand,string>(this, "transition_incall_termwait_BYE",
                                              (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_DialogVoip::a1001_incall_termwait_BYE,
                                              s_incall, s_termwait);



    new State_Transition<Sip_SMCommand,string>(this, "transition_incall_byerequest_hangup",
                                              (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand& )) &Sip_DialogVoip::a1002_incall_byerequest_hangup,
                                              s_incall, s_byerequest);

    new State_Transition<Sip_SMCommand,string>(this, "transition_byerequest_termwait_26",
                                              (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand& )) &Sip_DialogVoip::a1003_byerequest_termwait_26,
                                              s_byerequest,s_termwait);


    // Transaction/dialog management
    new State_Transition<Sip_SMCommand,string>(this, "transition_termwait_terminated_notransactions",
                                              (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_DialogVoip::a1101_termwait_terminated_notransactions,
                                              s_termwait, s_terminated);

    new State_Transition<Sip_SMCommand,string>(this, "transition_termwait_termwait_earlynotify",
                                              (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_DialogVoip::a1102_termwait_termwait_early,
                                              s_termwait, s_termwait);

    // Locally initiated call transfer
    new State_Transition<Sip_SMCommand,string>(this, "transition_incall_transferrequested_transfer",
                                              (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_DialogVoip::a1201_incall_transferrequested_transfer,
                                              s_incall, s_transferrequested);

    new State_Transition<Sip_SMCommand,string>(this, "transition_transferrequested_transferpending_202",
                                              (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_DialogVoip::a1202_transferrequested_transferpending_202,
                                              s_transferrequested, s_transferpending);

    new State_Transition<Sip_SMCommand,string>(this, "transition_transferrequested_incall_36",
                                              (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_DialogVoip::a1203_transferrequested_incall_36,
                                              s_transferrequested, s_incall);

    new State_Transition<Sip_SMCommand,string>(this, "transition_transferpending_transferpending_notify",
                                              (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_DialogVoip::a1204_transferpending_transferpending_notify,
                                              s_transferpending, s_transferpending);

    new State_Transition<Sip_SMCommand,string>(this, "transition_transferpending_termwait_BYE",
                                              (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_DialogVoip::a1001_incall_termwait_BYE,
                                              s_transferpending, s_termwait);

    new State_Transition<Sip_SMCommand,string>(this, "transition_transferpending_byerequest_hangup",
                                              (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_DialogVoip::a1002_incall_byerequest_hangup,
                                              s_transferpending, s_byerequest);


    // Remotely initiated call transfer
    new State_Transition<Sip_SMCommand,string>(this, "transition_incall_transferaskuser_REFER",
                                              (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_DialogVoip::a1301_incall_transferaskuser_REFER,
                                              s_incall, s_transferaskuser);

    new State_Transition<Sip_SMCommand,string>(this, "transition_transferaskuser_transferstarted_accept",
                                              (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_DialogVoip::a1302_transferaskuser_transferstarted_accept,
                                              s_transferaskuser, s_transferstarted);

    new State_Transition<Sip_SMCommand,string>(this, "transition_transferaskuser_incall_refuse",
                                              (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_DialogVoip::a1303_transferaskuser_incall_refuse,
                                              s_transferaskuser, s_incall);

    new State_Transition<Sip_SMCommand,string>(this, "transition_transferstarted_termwait_bye",
                                              (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_DialogVoip::a1001_incall_termwait_BYE,
                                              s_transferstarted, s_termwait);

    new State_Transition<Sip_SMCommand,string>(this, "transition_transferstarted_byerequest_hangup",
                                              (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_DialogVoip::a1002_incall_byerequest_hangup,
                                              s_transferstarted, s_byerequest);
}

Sip_Dialog_Voip::Sip_Dialog_Voip(SRef<Sip_Stack*> stack, SRef<Sip_Identity*> ident,
                                 bool usestun, SRef<Session *> mediasession, std::string cid )
{
    log_entry = NULL;
    set_up_state_machine();
    reflector_mode = false;
    participant_forwarder_mode  = true ;
    first_time_included = true;
    line_of_addition = -1;
    if(media_session)
        media_session->set_keyframe_request_callback(this);
}

Sip_Dialog_Voip::~Sip_Dialog_Voip()
{
    media_session->unregister();
}

void Sip_Dialog_Voip::send_bye(int)
{
    SRef<Sip_Request*> bye = create_sip_message_bye();

    SRef<Sip_Message*> pref(*bye);
    Sip_SMCommand cmd( pref, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE);
}

void Sip_Dialog_Voip::send_refer(int, const std::string referredUri)
{
    SRef<Sip_Request*> refer = create_sip_message_refer( referredUri );

    SRef<Sip_Message*> pref(*refer);
    Sip_SMCommand cmd( pref, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE );
}

void Sip_Dialog_Voip::send_cancel()
{
    my_assert( !last_invite.is_null());

    SRef<Sip_Request*> cancel = Sip_Request::create_sip_message_cancel( last_invite );

    add_route( cancel );

    cancel->get_header_value_from()->set_parameter("tag",_dialog_state._local_tag);
    // Don't include to-tag, allowing it to be forked by proxies.

    SRef<Sip_Message*> pref(*cancel);
    Sip_SMCommand cmd( pref, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    get_sip_stack()->enqueue_command( cmd, HIGH_PRIO_QUEUE );
}

void Sip_Dialog_Voip::send_refer_ok()
{
    SRef<Sip_Response*> ok= new Sip_Response( 202,"OK", lastRefer);
    ok->get_header_value_to()->set_parameter("tag",_dialog_state._local_tag);
    SRef<Sip_Header_Value *> contact =
            new Sip_Header_Value_Contact(
                get_dialog_config()->get_contact_uri(use_stun),
                -1); //set expires to -1, we do not use it (only in register)
    ok->add_header( new Sip_Header(*contact) );

    SRef<Sip_Message*> pref(*ok);
    Sip_SMCommand cmd( pref, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE );
}

void Sip_Dialog_Voip::send_bye_ok(SRef<Sip_Request*> bye)
{
    SRef<Sip_Response*> ok= new Sip_Response( 200,"OK", bye );
    ok->get_header_value_to()->set_parameter("tag",_dialog_state._local_tag);

    SRef<Sip_Message*> pref(*ok);
    Sip_SMCommand cmd( pref, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE );
}

void Sip_Dialog_Voip::send_notify_ok(SRef<Sip_Request*> notif)
{
    SRef<Sip_Response*> ok= new Sip_Response( 200, "OK", notif );
    ok->get_header_value_to()->set_parameter("tag",_dialog_state._local_tag);

    SRef<Sip_Message*> pref(*ok);
    Sip_SMCommand cmd( pref, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE );
}

void Sip_Dialog_Voip::send_info_ok(SRef<Sip_Request*> info)
{
    SRef<Sip_Response*> ok= new Sip_Response(200, "OK", info);
    ok->get_header_value_to()->set_parameter("tag", _dialog_state._local_tag);

    SRef<Sip_Message*> pref(*ok);
    Sip_SMCommand cmd(pref, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE );
}


void Sip_Dialog_Voip::send_refer_reject()
{
    SRef<Sip_Response*> forbidden = new Sip_Response( 403,"Forbidden", *lastRefer );
    forbidden->get_header_value_to()->set_parameter("tag",_dialog_state._local_tag);
    SRef<Sip_Message*> pref(*forbidden);
    Sip_SMCommand cmd( pref,Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE );
}

bool Sip_Dialog_Voip::handle_command(const Sip_SMCommand &command)
{
    my_dbg("signaling/sip") << "Sip_DialogVoip::handle_command got "<< c << endl;

    if (c.get_type()==Sip_SMCommand::COMMAND_STRING && _dialog_state._call_id.length()>0)
    {
        if (c.get_command_string().get_destination_id() != _dialog_state._call_id )
            return false;
    }

    if (c.get_type()==Sip_SMCommand::COMMAND_PACKET  && _dialog_state._call_id.length()>0)
    {
        if (c.get_command_packet()->get_call_id() != _dialog_state._call_id )
        {
            return false;
        }
        if (c.get_type()!=Sip_SMCommand::COMMAND_PACKET &&
                c.get_command_packet()->get_cseq()!= _dialog_state._seq_no)
        {
            return false;
        }

    }

    my_dbg("signaling/sip") << "Sip_DialogVoip::handle_command() sending command to Dialog: "<< c << endl;
    bool handled = Sip_Dialog::handle_command(c);

    if (!handled && c.get_type()==Sip_SMCommand::COMMAND_STRING && c.get_command_string().get_op()==Sip_Command_String::no_transactions)
    {
        return true;
    }

    if (c.get_type()==Sip_SMCommand::COMMAND_STRING && _dialog_state._call_id.length()>0)
    {
        if (!handled && c.get_command_string().get_destination_id() == _dialog_state._call_id )
        {
            my_dbg("signaling/sip") << "Warning: Sip_DialogVoIP ignoring command with matching call id"<< endl;
            return true;
        }
    }
    if (c.get_type()==Sip_SMCommand::COMMAND_PACKET && _dialog_state._call_id.length()>0)
    {
        if (!handled && c.get_command_packet()->get_call_id() == _dialog_state._call_id)
        {
            my_dbg("signaling/sip") << "Warning: Sip_DialogVoIP ignoring packet with matching call id"<< endl;
            return true;
        }
    }
    return handled;
}

SRef<Sip_Request*> Sip_Dialog_Voip::get_last_invite()
{
    return last_invite;
}

void Sip_Dialog_Voip::set_last_invite(SRef<Sip_Request*> i)
{
    last_invite = i;
}


SRef<Log_Entry *> Sip_Dialog_Voip::get_log_entry()
{
    return log_entry;
}

void Sip_Dialog_Voip::set_log_entry( SRef<Log_Entry *> l)
{
    this->log_entry = l;
}


void Sip_Dialog_Voip::set_media_session(SRef<Session*>s)
{
    media_session = s;
    if(media_session)
        media_session->set_keyframe_request_callback(this);
}

SRef<Session *> Sip_Dialog_Voip::get_media_session()
{
    return media_session;
}


bool Sip_Dialog_Voip::sort_mime(SRef<Sip_Message_Content *> Offer, std::string peerUri, int type)
{
    if (Offer)
    {
        if ( Offer->get_content_type().substr(0,9) == "multipart")
        {
            SRef<Sip_Message_Content *> part;
            part = ((Sip_Message_Content_Mime*)*Offer)->pop_first_part();
            while( *part != NULL)
            {
                sort_mime(part, peerUri, type);
                part = ((Sip_Message_Content_Mime*)*Offer)->pop_first_part();
            }
        }

        if( (Offer->get_content_type()).substr(0,15) == "application/sdp")
        {
            switch (type){
            case 10:
#if SIP_MEDIA_USE_MEDIAHANDLER_API
            {
                string sdpstr = Offer->get_string();
                Command_String cmd( _dialog_state._call_id, "set_sdp_offer", sdpstr );
                Command_String resp = get_sip_stack()->get_callback()->handle_command_resp( "media", cmd );
                if (resp.get_op()!="ok")
                    return false;
            }

#else

#ifdef ENABLE_TS
                ts.save("set_sdp_offer");
#endif
                if( !get_media_session()->set_sdp_offer( (Sdp_Packet*)*Offer, peerUri ) )
                    return false;
#ifdef ENABLE_TS
                ts.save("set_sdp_offer");
#endif

#endif
                return true;
            case 3:
#ifdef ENABLE_TS
                ts.save("set_sdp_answer");
#endif
                if( !get_media_session()->set_sdp_answer( (Sdp_Packet*)*Offer, peerUri ) ){
                    cerr << "!!!!!!!!!!!!SDP answer rejected ??" << endl;
                    return false;
                }
                /* charis :: storing the SDP packet that describes the multimedia session */

                get_media_session()->set_last_sdp_answer ((Sdp_Packet*)*Offer);
                get_media_session()->start();
#ifdef ENABLE_TS
                ts.save("set_sdp_answer");
#endif
                return true;
            default:
                std::merr << "No SDP match" << std::endl;
                return false;
            }
        }
    }
    return true;
}


void Sip_Dialog_Voip::send_reinvite(SRef <Sdp_Packet *> sdpPacket )
{
    _dialog_state._seq_no++;

    SRef<Sip_Request*> inv;
    string keyAgreementMessage;
    inv = Sip_Request::create_sip_message_invite(
                _dialog_state._call_id,
                Sip_Uri(_dialog_state._remote_target),
                get_dialog_config()->_sip_identity->get_sip_uri(),
                get_dialog_config()->get_contact_uri(use_stun),
                _dialog_state._seq_no,
                get_sip_stack() ) ;

    add_authorizations( inv );
    add_route( inv );

    /*
    changes packet version field not all ReInvites changes the version number
    the one we implemented does that because it describes different session each time
    */
    SRef <Sdp_HeaderO* > headerO = dynamic_cast<Sdp_HeaderO * > (*(sdpPacket->get_headers()[1]));
    int newVersionNumber  = get_media_session()->get_sdp_version_number();
    newVersionNumber++;
    get_media_session()->set_sdp_version_number(newVersionNumber);
    std::stringstream out;
    out << newVersionNumber;
    headerO->set_version(  out.str() );

    inv->set_content( *sdpPacket );

    inv->get_header_value_from()->set_parameter("tag",_dialog_state._local_tag );
    inv->get_header_value_to()->set_parameter("tag",_dialog_state._remote_tag);

    SRef<Sip_Message*> pktr(*inv);
    Sip_SMCommand scmd( pktr, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer );
    get_sip_stack()->enqueue_command(scmd, HIGH_PRIO_QUEUE);
    set_last_invite(inv);
}

void Sip_Dialog_Voip::send_ack()
{
    SRef<Sip_Request*> ack = create_sip_message_ack( get_last_invite() );

    // Send ACKs directly to the transport layer bypassing
    // the transaction layer
    Sip_SMCommand scmd( *ack, Sip_SMCommand::dialog_layer, Sip_SMCommand::transport_layer );

    get_sip_stack()->enqueue_command(scmd, HIGH_PRIO_QUEUE);
}

void Sip_Dialog_Voip::send_ack_sdp(SRef <Sdp_Packet *> p)
{
    SRef<Sip_Request*> ack = create_sip_message_ack( get_last_invite() );

    media_session->set_last_sdp_packet(p);
    media_session->set_last_sdp_answer(p);
    ack->set_content(*p);

    //cerr << "======================================== > charis sendAck woth packetsending ack with SDP packet \n";
    //cerr<< ack->get_string();
    // cerr<<" ========================================================================== Finished printing ACK =====\n";

    // Send ACKs directly to the transport layer bypassing
    // the transaction layer
    Sip_SMCommand scmd( *ack, Sip_SMCommand::dialog_layer, Sip_SMCommand::transport_layer );

    get_sip_stack()->enqueue_command(scmd, HIGH_PRIO_QUEUE);
}

void Sip_Dialog_Voip::try_requesting_video_keyframe()
{
    ++_dialog_state._seq_no; // this should be locked...
    //	cout << "Sip_DialogVoip::tryRequestingVideoKeyframe() called, calling createSip_MessageInfo for " << _dialog_state._call_id << ", " << _dialog_state._remote_uri << ", " << get_dialog_config()->_sip_identity->get_sip_uri() << ", " << _dialog_state._seq_no << endl;
    SRef<Sip_Dialog_Config*> dconf = get_dialog_config();
    if (dconf)
    {
        SRef<Sip_Request *> info = Sip_Request::create_sip_message_info(_dialog_state._call_id, _dialog_state._remote_uri, Sip_Uri(_dialog_state._remote_target), dconf->_sip_identity->get_sip_uri(), _dialog_state._seq_no);
        info->get_header_value_from()->set_parameter("tag", _dialog_state._local_tag);
        info->get_header_value_to()->set_parameter("tag", _dialog_state._remote_tag);
        Video_Fast_Update_Message_Content *content = new Video_Fast_Update_Message_Content();
        info->set_content(content);
        SRef<Sip_Message*> pref(*info);
        Sip_SMCommand scmd(pref, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
        get_sip_stack()->enqueue_command(scmd, HIGH_PRIO_QUEUE);
    }
}
