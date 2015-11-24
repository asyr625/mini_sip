#include "sip_dialog_voip_client.h"

#include "my_assert.h"
#include "sip_transition_utils.h"

#include "sip_header_warning.h"
#include "sip_header_contact.h"
#include "sip_header_from.h"
#include "sip_header_to.h"
#include "sip_header_require.h"
#include "sip_header_rseq.h"
#include "sip_message_content_mime.h"
#include "sip_message_content.h"
#include "sip_command_string.h"

#include "string_utils.h"
#include "base64.h"
#include "timestamp.h"
#include "termmanip.h"
#include "dbg.h"
#include "sip_smcommand.h"
#include "log_entry.h"

#include <time.h>
#include <iostream>
using namespace std;

#ifdef _WIN32_WCE
#	include "minisip_wce_extra_includes.h"
#endif

#define SIP_MEDIA_USE_HANDLECOMMAND_API 1

bool Sip_Dialog_Voip_Client::a2001_start_calling_invite( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::invite, Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer))
    {
#ifdef ENABLE_TS
        ts.save("a0_start_callingnoauth_invite");
#endif
        ++_dialog_state._seq_no;

        //set an "early" remoteUri ... we will update this later
        _dialog_state._remote_uri= command.get_command_string().get_param();

        send_invite();

        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Client::a2002_calling_calling_18X( const Sip_SMCommand &command)
{
    if (transition_match_sip_response("INVITE", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer,
                                      "18*"))
    {
        SRef<Sip_Response*> resp= (Sip_Response*) *command.get_command_packet();

#ifdef ENABLE_TS
        ts.save( RINGING );
#endif
        bool doPrack = false;
        if (resp->requires("100rel") && resp->get_status_code()!=100)
        {
            if( !handle_rel1xx( resp ) )
            {
                // Ignore retransmission
                // 				cerr << "Ignore 18x retransmission" << endl;
                return true;
            }
            doPrack = true;
        }
        else{
            //We must maintain the dialog state.
            _dialog_state.update_state( resp );
        }

        // Build peer uri used for authentication from remote uri,
        // but containing user and host only.
        Sip_Uri peer(_dialog_state._remote_uri);
        string peerUri = peer.get_protocol_id() + ":" + peer.get_user_ip_string();
        SRef<Sip_Message_Content *> content = resp->get_content();
        if( !content.is_null() )
        {
            SRef<Sdp_Packet*> sdp((Sdp_Packet*)*content);
            //Early media
            if( !sort_mime( content , peerUri, 3) )
            {
                // MIKEY failed
                // TODO reason header
                send_cancel();

                get_media_session()->stop();
                signal_if_no_transactions();
                // Skip prack
                // cerr << "Send cancel, skip prack" << endl;
                return true;
            }
        }

        if( doPrack )
            send_prack(resp);

        if( resp->get_status_code() == 180 )
        {
            Command_String cmdstr(_dialog_state._call_id, Sip_Command_String::remote_ringing, get_media_session()->get_peer_uri(), (get_media_session()->is_secure()?"secure":"unprotected"));
            get_sip_stack()->get_callback()->handle_Command("gui", cmdstr);
        }
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Client::a2003_calling_calling_1xx( const Sip_SMCommand &command)
{
    if (transition_match_sip_response("INVITE", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer, "1**"))
    {
        _dialog_state.update_state( SRef<Sip_Response*>((Sip_Response *)*command.get_command_packet()) );

        SRef<Sip_Response*> resp = (Sip_Response*)*command.get_command_packet();
        if (resp->requires("100rel") && resp->get_status_code()!=100)
        {
            if( !handle_rel1xx( resp ) )
            {
                // Ignore retransmission
                // 				cerr << "Ignore 1xx retransmission" << endl;
                return true;
            }

            send_prack(resp);
        }

        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Client::a2004_calling_incall_2xx( const Sip_SMCommand &command)
{
    if (transition_match_sip_response("INVITE", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer,
                                      "2**"))
    {
#ifdef ENABLE_TS
        ts.save("a3_callingnoauth_incall_2xx");
#endif
        //		std::cout << "Sip_Dialog_Voip_Client::a2004_calling_incall_2xx() called at " << mtime() << "ms" << std::endl;
        SRef<Sip_Response*> resp(  (Sip_Response*)*command.get_command_packet() );

        // Build peer uri used for authentication from remote uri,
        // but containing user and host only.
        Sip_Uri peer(_dialog_state._remote_uri);
        string peerUri = peer.get_protocol_id() + ":" + peer.get_user_ip_string();
        _dialog_state.update_state( resp );

        set_log_entry( new Log_Entry_Outgoing_Completed_Call() );
        get_log_entry()->start = time( NULL );
        get_log_entry()->peer_sip_uri = peerUri;


        //FIXME: CESC: for now, route set is updated at the transaction layer

        bool ret = sort_mime(*resp->get_content(), peerUri, 3);
        if( !ret ) {
            // Fall through to a2012 terminating the
            // call and sending security_failed to the gui
            return false;
        }
        send_ack();

        Command_String cmdstr(_dialog_state._call_id, Sip_Command_String::invite_ok, get_media_session()->get_peer_uri(),
                              (get_media_session()->is_secure()?"secure":"unprotected"));

        get_sip_stack()->get_callback()->handle_Command("gui", cmdstr);

        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Client::a2005_calling_termwait_CANCEL(const Sip_SMCommand &command)
{
    if (transition_match("CANCEL", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer))
    {
        get_media_session()->stop();
        signal_if_no_transactions();
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Client::a2006_calling_termwait_cancel(const Sip_SMCommand &command)
{
    if ( transition_match(command, Sip_Command_String::cancel, Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer)
         || transition_match(command, Sip_Command_String::hang_up, Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer))
    {
        send_cancel();
        get_media_session()->stop();
        signal_if_no_transactions();
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Client::a2007_calling_termwait_36( const Sip_SMCommand &command)
{
    if (transition_match_sip_response("INVITE", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer, "3**\n4**\n5**\n6**"))
    {
        _dialog_state._is_terminated = true;

        SRef<Log_Entry *> rejectedLog( new Log_Entry_Call_Rejected() );
        rejectedLog->start = time( NULL );
        rejectedLog->peerSip_Uri = _dialog_state._remote_tag;

        SRef<Sip_Response*> resp = (Sip_Response*)*command.get_command_packet();

        if (sip_response_filter_match(resp,"404"))
        {
            Command_String cmdstr(_dialog_state._call_id, Sip_Command_String::remote_user_not_found);
            assert(get_sip_stack());
            assert(get_sip_stack()->get_callback());
            get_sip_stack()->get_callback()->handle_Command("gui", cmdstr);
            ((Log_Entry_Failure *)*rejectedLog)->error =
                    "User not found";
            set_log_entry( rejectedLog );
            rejectedLog->handle();

        }
        else if (sip_response_filter_match(resp,"606"))
        {
            ((Log_Entry_Failure *)*rejectedLog)->error =
                    "User could not handle the call";
            set_log_entry( rejectedLog );
            rejectedLog->handle();

            Command_String cmdstr( _dialog_state._call_id, Sip_Command_String::remote_unacceptable, command.get_command_packet()->getWarningMessage());
            get_sip_stack()->get_callback()->handle_Command( "gui", cmdstr );
        }
        else if (sip_response_filter_match(resp,"3**") ||
                 sip_response_filter_match(resp,"4**") ||
                 sip_response_filter_match(resp,"5**") ||
                 sip_response_filter_match(resp,"6**"))
        {
            ((Log_Entry_Failure *)*rejectedLog)->error =
                    "User rejected the call";
            set_log_entry( rejectedLog );
            rejectedLog->handle();
            Command_String cmdstr( _dialog_state._call_id, Sip_Command_String::remote_reject);
            get_sip_stack()->get_callback()->handle_Command( "gui",cmdstr );
        }

        get_media_session()->stop();
        signal_if_no_transactions();
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Client::a2008_calling_calling_40X( const Sip_SMCommand &command)
{
    if (transition_match_sip_response("INVITE", command,Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer, "407\n401"))
    {
        SRef<Sip_Response*> resp( (Sip_Response*)*command.get_command_packet() );

        _dialog_state.update_state( resp ); //nothing will happen ... 4xx responses do not update ...

        if( !update_authentications( resp ) )
        {
            // Fall through to a2007_calling_termwait_36
            return false;
        }
        ++_dialog_state._seq_no;
        send_invite();
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Client::a2012_calling_termwait_2xx(const Sip_SMCommand &command)
{
    if (transition_match_sip_response("INVITE", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer, "2**")){

        ++_dialog_state._seq_no;

        send_bye(_dialog_state._seq_no);

        Command_String cmdstr(_dialog_state._call_id, Sip_Command_String::security_failed);
        get_sip_stack()->get_callback()->handle_Command("gui", cmdstr);

        get_media_session()->stop();
        signal_if_no_transactions();
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Client::a2013_calling_termwait_transporterror( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::transport_error, Sip_SMCommand::transaction_layer,
                         Sip_SMCommand::dialog_layer ))
    {
        Command_String cmdstr(_dialog_state._call_id, Sip_Command_String::transport_error);
        get_sip_stack()->get_callback()->handle_Command("gui",cmdstr);
        signal_if_no_transactions();
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Client::a2017_any_any_2XX( const Sip_SMCommand &command)
{
    if (transition_match_sip_response("INVITE", command, Sip_SMCommand::transport_layer, Sip_SMCommand::dialog_layer,
                                      "2**"))
    {
        string state = get_current_state_name();

        if( state == "terminated" )
            return false;

        SRef<Sip_Message*> pack = command.get_command_packet();

        if( _dialog_state._remote_tag != pack->get_header_value_to()->get_parameter("tag") )
        {
            // Acknowledge and terminate 2xx from other fork
            send_ack();
            send_sip_message( *create_sip_message_bye() );
        }
        else if( state != "termwait" )
        {
            send_ack();
        }
        return true;
    }
    return false;
}

void Sip_Dialog_Voip_Client::set_up_state_machine()
{
    State<Sip_SMCommand,string> *s_start=new State<Sip_SMCommand,string>(this,"start");
    add_state(s_start);

    State<Sip_SMCommand,string> *s_calling=new State<Sip_SMCommand,string>(this,"calling");
    add_state(s_calling);

    SRef<State<Sip_SMCommand,string> *> s_incall = get_state("incall");
    SRef<State<Sip_SMCommand,string> *> s_termwait= get_state("termwait");

    new State_Transition<Sip_SMCommand,string>(this, "transition_any_any_2XX",
                                               (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Client::a2017_any_any_2XX,
                                               anyState, anyState);




    new State_Transition<Sip_SMCommand,string>(this, "transition_start_calling_invite",
                                               (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Client::a2001_start_calling_invite,
                                               s_start, s_calling);

    new State_Transition<Sip_SMCommand,string>(this, "transition_calling_calling_18X",
                                               (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Client::a2002_calling_calling_18X,
                                               s_calling, s_calling);

    new State_Transition<Sip_SMCommand,string>(this, "transition_calling_calling_1xx",
                                               (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Client::a2003_calling_calling_1xx,
                                               s_calling, s_calling);

    new State_Transition<Sip_SMCommand,string>(this, "transition_calling_incall_2xx",
                                               (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Client::a2004_calling_incall_2xx,
                                               s_calling, s_incall);

    // Must be added after the calling->incall transition since this is
    // the "fallback one" if we don't accept the 2XX reply (for example
    // authentication error)
    new State_Transition<Sip_SMCommand,string>(this, "transition_calling_termwait_2xx",
                                               (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Client::a2012_calling_termwait_2xx,
                                               s_calling, s_termwait);

    new State_Transition<Sip_SMCommand,string>(this, "transition_calling_termwait_CANCEL",
                                               (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Client::a2005_calling_termwait_CANCEL,
                                               s_calling, s_termwait);

    new State_Transition<Sip_SMCommand,string>(this, "transition_calling_termwait_cancel",
                                               (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Client::a2006_calling_termwait_cancel,
                                               s_calling, s_termwait);

    new State_Transition<Sip_SMCommand,string>(this, "transition_calling_calling_40X",
                                               (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Client::a2008_calling_calling_40X,
                                               s_calling, s_calling);

    new State_Transition<Sip_SMCommand,string>(this, "transition_calling_termwait_36",
                                               (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Client::a2007_calling_termwait_36,
                                               s_calling, s_termwait);

    new State_Transition<Sip_SMCommand,string>(this, "transition_calling_termwait_transporterror",
                                               (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Client::a2013_calling_termwait_transporterror,
                                               s_calling, s_termwait);


    set_current_state(s_start);
}

Sip_Dialog_Voip_Client::Sip_Dialog_Voip_Client(SRef<Sip_Stack*> stack, SRef<Sip_Identity*> ident,
                                               bool stun, bool anat, SRef<Session *> s, std::string cid)
    : Sip_Dialog_Voip(stack, ident, stun, s, cid),
      use_anat(anat)
{
    set_up_state_machine();
}

Sip_Dialog_Voip_Client::~Sip_Dialog_Voip_Client()
{

}

void Sip_Dialog_Voip_Client::send_invite()
{
    SRef<Sip_Request*> inv;
    string keyAgreementMessage;
    inv = Sip_Request::create_sip_message_invite( _dialog_state._call_id, Sip_Uri(_dialog_state._remote_uri),
                                                  get_dialog_config()->_sip_identity->get_sip_uri(),
                                                  get_dialog_config()->get_contact_uri(use_stun),
                                                  _dialog_state._seq_no,
                                                  get_sip_stack() ) ;

    add_authorizations( inv );
    add_route( inv );

    /* Get the session description from the Session */

    //There might be so that there are no SDP. Check!
    SRef<Sdp_Packet *> sdp;
    if (media_session)
    {
        // Build peer uri used for authentication from remote uri,
        // but containing user and host only.
        Sip_Uri peer(_dialog_state._remote_uri);
        string peerUri = peer.get_protocol_id() + ":" + peer.get_user_ip_string();

#if SIP_MEDIA_USE_HANDLECOMMAND_API

#ifdef ENABLE_TS
        ts.save("get_sdp_offer");
#endif
        Command_String cmd( _dialog_state._call_id, "get_sdp_offer" );
        Command_String resp = get_sip_stack()->get_callback()->handle_command_resp( "media", cmd );
        sdp = new Sdp_Packet(resp.get_op());
#ifdef ENABLE_TS
        ts.save("get_sdp_offer");
#endif

#else

#ifdef ENABLE_TS
        ts.save("get_sdp_offer");
#endif
        bool anat =  use_anat;
        sdp = media_session->get_sdp_offer( peerUri, anat );
#ifdef ENABLE_TS
        ts.save("get_sdp_offer");
#endif

#endif
        if( !sdp )
        {
            // FIXME: this most probably means that the
            // creation of the MIKEY message failed, it
            // should not happen
            my_err << "Sdp was NULL in send_invite" << endl;
            return;
        }

        if( !sdp->get_session_level_connection( "group" ).empty() )
        {
            inv->add_header(new Sip_Header(new Sip_Header_Value_Require("sdp-anat")));
        }
    }

    /* Add the latter to the INVITE message */ // If it exists


    //-------------------------------------------------------------------------------------------------------------//
    inv->set_content( *sdp );
    //-------------------------------------------------------------------------------------------------------------//

    inv->get_header_value_from()->set_parameter("tag",_dialog_state._local_tag );

    //	ts.save( INVITE_END );
    SRef<Sip_Message*> pktr(*inv);

    Sip_SMCommand scmd( pktr, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer );

    get_sip_stack()->enqueue_command(scmd, HIGH_PRIO_QUEUE);
    set_last_invite(inv);
}

void Sip_Dialog_Voip_Client::send_ack()
{
    SRef<Sip_Request*> ack = create_sip_message_ack( get_last_invite() );

    // Send ACKs directly to the transport layer bypassing
    // the transaction layer
    Sip_SMCommand scmd( *ack, Sip_SMCommand::dialog_layer, Sip_SMCommand::transport_layer );

    get_sip_stack()->enqueue_command(scmd, HIGH_PRIO_QUEUE);
}

void Sip_Dialog_Voip_Client::send_prack(SRef<Sip_Response*>)
{
    SRef<Sip_Request*> prack = create_sip_message_prack( rel100resp );
    send_sip_message( *prack );
}

bool Sip_Dialog_Voip_Client::handle_rel1xx( SRef<Sip_Response*> resp )
{
    SRef<Sip_Header_Value *> value = resp->get_header_value_no( SIP_HEADER_TYPE_RSEQ, 0 );

    if( !value )
        return false;

    SRef<Sip_Header_Value_RSeq *> rseq = dynamic_cast<Sip_Header_Value_RSeq*>( *value );
    uint32_t rseqNo = rseq->get_rseq();

    // First reliable provisional response
    // Next in-order reliable provisional response
    if( !(_dialog_state._rseq_no == (uint32_t)-1 || rseqNo > _dialog_state._rseq_no ) )
        return false;

    _dialog_state.update_state( resp );
    _dialog_state._seq_no++;
    _dialog_state._rseq_no = rseqNo;

    return true;
}
