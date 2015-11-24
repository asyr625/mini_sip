#include "sip_dialog_voip_server.h"

#include "my_assert.h"
#include "sip_transition_utils.h"
#include "sip_command_string.h"

#include "sip_header_warning.h"
#include "sip_header_contact.h"
#include "sip_header_from.h"
#include "sip_header_rack.h"
#include "sip_header_route.h"
#include "sip_header_require.h"
#include "sip_header_rseq.h"
#include "sip_header_to.h"

#include "sip_message_content_mime.h"
#include "sip_message_content.h"
#include "sip_smcommand.h"
#include "log_entry.h"

#include "string_utils.h"
#include "base64.h"
#include "timestamp.h"
#include "termmanip.h"
#include "dbg.h"

#include <time.h>
#include <iostream>
using namespace std;

#ifdef _WIN32_WCE
#	include "minisip_wce_extra_includes.h"
#endif


/*

 The responsibility of the voip invite server is to accept
 or reject an incoming call request. It should either end
 up in the in_call state or be terminated. The super class,
 SipDialogVoip handles calls after they have been established.

 This class adds two states, start and ringing. The in_call,
 termwait and terminated states are inherited from the super-class
 (they have dotted borders in the diagram below).

 The "dotted" states are implemented in SipDialogVoip.cxx.

 INVITE & !100rel
 a3001:transIR;  +---------------+									received invite without an offer and support 100rel
       send 180  |               |----------------------------------------------------------------------------------------------+
      +----------|     start     |		received Invite without SDP  a3011 ( no100rel)					|a30077 : send 183 with SDP offer
      |          |               |--------------------------------------------------------------+				v  and Mikey (if  security)
      |          +---------------+			send: 180 				|			+----------------+
      |                  |									|			|		 |
      |                  | INVITE && supports 100rel						|			| 100relNoOffer	 |
      |                  | a3007:transIR; send 183						|		  +----	|		 |
      |                  |									|		  |	+----------------+
      |                  V				error in MIKEY in Prack	 a3111/a3015/a3004		  |		|
      |          +---------------+	+-------------------------------------------------------------------------+		| received PRACK with SDP answer
      |          |               |	|							|				| with proper MIKEY going to incall
      |          |    100rel     |------+---+							v				|  a30088 :send PRACK 200 Ok and 180 ringing
      |          |               |          |						+---------------+			|
      |          +---------------+          |		a3015//a3004			|emptyInvite	|			|
      |                  | PRACK & 100rel   +-------------------------------------------| ( ringing )	|<----------------------+
      |                  | a3008: send 200  |		received Bye or Cancel		+---------------+
      |                  |        send 180  |							|send 200 Ok (with previous Mikey)  a3013
      |                  V                  |							V
      |          +---------------+          |						+---------------+
      |          |               |          | 						|		|
      +--------->|    ringing    |----------+  reject					| waitingACK	|---------------------------------------+
                 |               |          |  a3005:send40X				|		|					|
                 +---------------+	    |						+---------------+					|
                         |                  |  CANCEL						|						|
                         | accept_invite &  |  a3004:transCancelResp				|						|
                         | authok           |							| received ACK with SDP answer			|
                 | a3002: send 200  |  BYE						| start the media go to incall			|
                 V                  |  a3003:transByeResp				| a3014						|
                 +. . . . . . . .+ <------------------------------------------------------------+						|
                 .               .          |  INVITE & a11 fail										|
                 .   in call     .          |  a3006: send 606											|
                 .               .          |													|
                 +. . . . . . . .+          |													|
                                            |				received Cancel/Bye instead of an ACK 	a3004 /a3111/a3015		|
                                            +---------------------------------------------------------------------------------------------------+
                                            |				going to termwait state
                 +. . . . . . . .+          |
                 .               .          |
                 .   termwait    .<---------+
                 .               .
                 +. . . . . . . .+



                 +. . . . . . . .+
                 .               .
                 .  terminated   .
                 .               .
                 + . . . . . . . +


                 +---------------+
           +---->|               |<----+  ResendTimer1xx
           |     |      ANY      |     |  a3009: resend 1XX
           +-----|               |-----+
 PRACK & 100rel  +---------------+
 a3010: send 200 &
        cancel timer

*/

bool Sip_Dialog_Voip_Server::a3111_any_termwait_error( const Sip_SMCommand &command)
{
    if ( error_mode )
    {
        Command_String cmdstr(_dialog_state._call_id, Sip_Command_String::remote_hang_up);
        get_sip_stack()->get_callback()->handle_command("gui", cmdstr);

        get_media_session()->stop();
        signal_if_no_transactions();
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Server::a3015_waitingACK_termwait_BYE( const Sip_SMCommand &command)
{
    if (transition_match("BYE", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer) &&
            _dialog_state._remote_tag != "")
    {
        SRef<Sip_Request*> bye = (Sip_Request*) *command.get_command_packet();
        send_bye_ok( bye );

        Command_String cmdstr(_dialog_state._call_id, Sip_Command_String::remote_hang_up);
        get_sip_stack()->get_callback()->handle_command("gui", cmdstr);

        get_media_session()->stop();
        signal_if_no_transactions();
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Server::a3011_start_emptyInvite_emptyInvite(const Sip_SMCommand &command)
{
    if (transition_match("INVITE", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer))
    {
        SRef<Sip_Request*> inv = (Sip_Request *)*command.get_command_packet();
        if( inv->supported("100rel") ||   inv->requires("100rel") )
        {
            return false;
        }
        if ( (inv->get_content() ) ){return false;}
        set_last_invite(inv);
        _dialog_state.update_state( inv );
        Sip_Uri peer(_dialog_state._remote_uri);
        string peerUri = peer.get_protocol_id() + ":" + peer.get_user_ip_string();

        ///bool anat = false;//ask Erik

        //sdpNoOffer = media_session->get_sdp_offer( peerUri, anat );
        //send_ringing(sdpNoOffer);
        send_ringing();
        Command_String cmdstr(_dialog_state._call_id, Sip_Command_String::incoming_available, peerUri,
                             (get_media_session()->is_secure()?"secure":"unprotected") );

        cmdstr["touri"]=inv->get_header_value_to()->get_uri().get_string();
        get_sip_stack()->get_callback()->handle_command("gui", cmdstr );
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Server::a3014_waitingACK_incall_arrivedACK(const Sip_SMCommand &command)
{
    if ( transition_match("ACK", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer) )
    {
        SRef<Sip_Request*> ack = (Sip_Request *)*command.get_command_packet();
        Sip_Uri peer(_dialog_state._remote_uri);
        string peerUri = peer.get_protocol_id() + ":" + peer.get_user_ip_string();

        if ( checked_in_ringing && ack->get_content() )
        {
            send_bye(_dialog_state._seq_no);
            error_mode = true;
            return false;
        }

        if ( ! checked_in_ringing )
        {
            if ( ! ack->get_content() )
            {
                send_bye(_dialog_state._seq_no);
                error_mode = true;
                return false;
            }
            bool ret = sort_mime(*ack->get_content(), peerUri, 3);
            if ( ret == false )
            {
                send_bye(_dialog_state._seq_no);
                error_mode = true;
                return false;
            }
        }
        // starting session
        get_media_session()->start();
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Server::a3013_emptyInvite_waitingACK_USERACCEPTED(const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::accept_invite, Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer))
    {
        // get SDP offer
        Sip_Uri peer(_dialog_state._remote_uri);
        string peerUri = peer.get_protocol_id() + ":" + peer.get_user_ip_string();

        bool anat =  false;//todo ask ERIK

        if( !checked_in_ringing ){
            // we create a new offer thereis a chance that we had created that new offer previously when sending provisional responces
            sdp_no_offer = media_session->get_sdp_offer( peerUri, anat );
            send_invite_ok( sdp_no_offer );
        }else {
            // sending empty 200 Ok packet
            send_invite_ok(NULL);
        }

        Command_String cmdstr(_dialog_state._call_id, Sip_Command_String::invite_ok, peerUri,
                             (get_media_session()->is_secure()?"secure":"unprotected"),
                             "NO_PARTICIPANT_SUPPORT" );
        get_sip_stack()->get_callback()->handle_command("gui", cmdstr );

        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Server::a30077_start_100relNoOffer_INVITENOFFER( const Sip_SMCommand &command)
{
    if( !transition_match("INVITE", command,Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer)
            // ||  !get_sip_stack()->get_stack_config()->use100_rel
            ) {
        return false;
    }
    SRef<Sip_Request*> inv = (Sip_Request *)*command.get_command_packet();
    if (  ( inv->get_content() ) ){ return false;}
    if( !inv->supported("100rel") ){  return false;}
    if( reject_unsupported( inv ) ){return false;}


    use100_rel = true;
    resend_timer1xx = get_sip_stack()->get_timers()->getA();
    request_timeout(resend_timer1xx,"ResendTimer1xx");
    _dialog_state._rseq_no = rand() % (1<<31);
    set_last_invite(inv);
    _dialog_state.update_state( get_last_invite() );
    // Build peer uri used for authentication from remote uri,
    // but containing user and host only.
    Sip_Uri peer(_dialog_state._remote_uri);
    string peerUri = peer.get_protocol_id() + ":" + peer.get_user_ip_string();
    checked_in_ringing = true;
    sdp_no_offer = media_session->get_sdp_offer();
    send_session_progress(sdp_no_offer);
    return true;
}

bool Sip_Dialog_Voip_Server::a3007_start_100rel_INVITE( const Sip_SMCommand &command)
{
    if( !transition_match("INVITE", command,Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer) //  !get_sip_stack()->get_stack_config()->use100_rel
            ) {
        return false;
    }
    cerr<< " trying to match invite with SDP offer !!!!\n"	;
    SRef<Sip_Request*> inv = (Sip_Request *)*command.get_command_packet();
    if ( ! ( inv->get_content() ) )
         return false;
    if( reject_unsupported( inv ) ){// Unsupported extension(s)
        return true;
    }
    if( /*!inv->supported("100rel") && */!inv->requires("100rel") ){
        return false;
    }
    use100_rel = true;
    resend_timer1xx = get_sip_stack()->get_timers()->getA();
    request_timeout(resend_timer1xx,"ResendTimer1xx");
    _dialog_state._rseq_no = rand() % (1<<31);
    set_last_invite(inv);
    _dialog_state.update_state( get_last_invite() );
    // Build peer uri used for authentication from remote uri,
    // but containing user and host only.
    Sip_Uri peer(_dialog_state._remote_uri);
    string peerUri = peer.get_protocol_id() + ":" + peer.get_user_ip_string();
    if ( (*command.get_command_packet()->get_content()) )
    {
        if(!sort_mime(*command.get_command_packet()->get_content(), peerUri, 10))
        {
            my_err << "No MIME match" << endl;
            return false;
        }
    }
    send_session_progress();
    return true;
}

bool Sip_Dialog_Voip_Server::a3001_start_ringing_INVITE( const Sip_SMCommand &command)
{
    if (transition_match("INVITE", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer))
    {
        SRef<Sip_Request*> inv = (Sip_Request *)*command.get_command_packet();

        if ( get_dialog_config()->_sip_identity->get_sip_uri().get_user_name() != inv->get_header_value_to()->get_uri().get_user_name()  )
            return false;

        if ( ! ( inv->get_content() ) )
             return false;


        if( inv->requires("100rel") /* || inv->supported( "100rel")*/ ){
            // TODO reject unsupported extension
            return false;
        }
        set_last_invite(inv);
        _dialog_state.update_state( inv );

        // Build peer uri used for authentication from remote uri,
        // but containing user and host only.
        Sip_Uri peer(_dialog_state._remote_uri);
        string peerUri = peer.get_protocol_id() + ":" + peer.get_user_ip_string();
        if ( ( inv->get_content() ) )
        {
            if(!sort_mime(*inv->get_content(), peerUri, 10))
            {
                my_err << "No MIME match" << endl;
                return false;
            }
        }else{
            ;
        }

        /*		SRef<Sip_Header_Value*> identity = command.get_command_packet()->get_header_value_no(SIP_HEADER_TYPE_IDENTITY, 0);
        SRef<Sip_Header_Value*> identityinfo = command.get_command_packet()->get_header_value_no(SIP_HEADER_TYPE_IDENTITYINFO, 0);

        bool identityVerified=false;
        if (identity && identityinfo){
            cerr << "IDENTITY: found identity and identity-info header values"<< endl;
            assert(dynamic_cast<Sip_Header_Value_Identity*>( *identity));
            assert(dynamic_cast<Sip_Header_Value_IdentityInfo*>( *identityinfo));
            SRef<Sip_Header_Value_Identity*> ident = (Sip_Header_Value_Identity*) *identity;
            SRef<Sip_Header_Value_Identity*> identinfo = (Sip_Header_Value_Identity*) *identityinfo;

            cerr << "IDENTITY: algorithm is: <"<< identinfo->getParameter("alg") << ">"<< endl;

            //downloadCertificate( identinfo->getCertUri() );

            identityVerified = verifyIdentityHeader(ident);


            //TODO: check that the identity is rsa-sha1

            if (!identityVerified){
#ifdef DEBUG_OUTPUT
                cerr << "IDENTITY: the verification FAILED!"<< endl;
#endif
            }


        }else{
            cerr << "IDENTITY: did not find identity header value"<< endl;
        }
*/
        Command_String cmdstr(_dialog_state._call_id,
                             Sip_Command_String::incoming_available,
                             peerUri,
                             (get_media_session()->is_secure()?"secure":"unprotected")
                             );
        cmdstr["touri"]=inv->get_header_value_to()->get_uri().get_string();
        get_sip_stack()->get_callback()->handle_command("gui", cmdstr );

        send_ringing();

        if( get_sip_stack()->get_stack_config()->auto_answer )
        {
            Command_String accept( _dialog_state._call_id, Sip_Command_String::accept_invite );
            Sip_SMCommand sipcmd(accept, Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer);
            get_sip_stack()->enqueue_command(sipcmd,HIGH_PRIO_QUEUE);
        }
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Server::a3002_ringing_incall_accept( const Sip_SMCommand &command)
{
    if (transition_match(command, Sip_Command_String::accept_invite, Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer))
    {
#ifdef ENABLE_TS
        ts.save(USER_ACCEPT);
#endif
        Command_String cmdstr(_dialog_state._call_id,
                             Sip_Command_String::invite_ok,
                             get_media_session()->get_peer_uri(),
                             (get_media_session()->is_secure()?"secure":"unprotected")
                             );
        get_sip_stack()->get_callback()->handle_command("gui", cmdstr );

        my_assert( !get_last_invite().isNull() );

        send_invite_ok();

        get_media_session()->start();

        SRef<Log_Entry *> log= new Log_Entry_Incoming_Completed_Call();

        log->start = time( NULL );
        log->peer_sip_uri = get_last_invite()->get_from().get_string();

        set_log_entry( log );
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Server::a3003_ringing_termwait_BYE( const Sip_SMCommand &command)
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

        send_bye_ok( bye );

        Command_String cmdstr(_dialog_state._call_id, Sip_Command_String::remote_hang_up);
        get_sip_stack()->get_callback()->handle_command("gui", cmdstr);

        get_media_session()->stop();

        signal_if_no_transactions();
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Server::a3004_ringing_termwait_CANCEL( const Sip_SMCommand &command)
{
    if (transition_match("CANCEL", command, Sip_SMCommand::transaction_layer,
                        Sip_SMCommand::dialog_layer))
    {
        SRef<Sip_Request*> cancel =
                (Sip_Request*)*command.get_command_packet();

        ///		const string branch = cancel->getFirstViaBranch();
        ///
        ///		if (get_last_invite()->getFirstViaBranch() != branch)
        ///			return false;

        // Send 487 Request Cancelled for INVITE
        SRef<Sip_Response*> cancelledResp = new Sip_Response( 487, "Request Cancelled", *get_last_invite() );
        cancelledResp->get_header_value_to()->set_parameter("tag",_dialog_state._local_tag);
        SRef<Sip_Message*> cancelledMsg(*cancelledResp);
        Sip_SMCommand cancelledCmd( cancelledMsg, Sip_SMCommand::dialog_layer,
                                   Sip_SMCommand::transaction_layer);
        get_sip_stack()->enqueue_command(cancelledCmd, HIGH_PRIO_QUEUE);

        // Send 200 OK for CANCEL
        SRef<Sip_Response*> okResp = new Sip_Response( 200,"OK", cancel );
        okResp->get_header_value_to()->set_parameter("tag",_dialog_state._local_tag);
        SRef<Sip_Message*> okMsg(*okResp);
        Sip_SMCommand okCmd( okMsg, Sip_SMCommand::dialog_layer,
                            Sip_SMCommand::transaction_layer);
        //cr->handle_command(okCmd);
        get_sip_stack()->enqueue_command(okCmd, HIGH_PRIO_QUEUE);

        /* Tell the GUI */
        Command_String cmdstr(_dialog_state._call_id, Sip_Command_String::remote_cancelled_invite,"");
        get_sip_stack()->get_callback()->handle_command("gui", cmdstr );

        get_media_session()->stop();
        signal_if_no_transactions();
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Server::a3005_ringing_termwait_reject( const Sip_SMCommand &command)
{
    if ( transition_match(command, Sip_Command_String::reject_invite, Sip_SMCommand::dialog_layer,
                                Sip_SMCommand::dialog_layer) ||
                transition_match(command, Sip_Command_String::hang_up, Sip_SMCommand::dialog_layer,
                                Sip_SMCommand::dialog_layer))
    {
        send_reject();

        get_media_session()->stop();
        signal_if_no_transactions();
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Server::a3006_start_termwait_INVITE( const Sip_SMCommand &command)
{
    if (transition_match("INVITE", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer))
    {
        SRef<Sip_Request*> inv = (Sip_Request *)*command.get_command_packet();

        set_last_invite( inv );
        _dialog_state.update_state( get_last_invite() );

        if ( get_dialog_config()->_sip_identity->get_sip_uri().get_user_name() != inv->get_header_value_to()->get_uri().get_user_name()  )
            send_user_not_found();
        else
            send_not_acceptable( );

        signal_if_no_transactions();
        return true;
    }
    return false;
}

bool Sip_Dialog_Voip_Server::is_matching_prack( SRef<Sip_Message*> provisional, SRef<Sip_Request*> prack )
{
    SRef<Sip_Header_Value *> value = prack->get_header_value_no( SIP_HEADER_TYPE_RACK, 0 );
    if( !value )
    {
        // TODO reject 481
        cerr << "No RAck" << endl;
        return false;
    }

    SRef<Sip_Header_Value_RAck *> rack = dynamic_cast<Sip_Header_Value_RAck*>(*value);
    if( !rack )
    {
        // TODO reject 481
        cerr << "Bad RAck" << endl;
        return false;
    }

    if( rack->get_method() != provisional->get_cseq_method() ||
            rack->get_cseq_num() != provisional->get_cseq() ||
            (unsigned)rack->get_response_num() != _dialog_state._rseq_no )
    {
        // TODO reject 481
        cerr << "Non matching RAck" << endl;
        return false;
    }
    return true;
}

bool Sip_Dialog_Voip_Server::a30088_100relNoOffer_emptyInvite_PRACK( const Sip_SMCommand &command)
{
    if( ! (use100_rel && last_provisional &&transition_match("PRACK",  command,Sip_SMCommand::transaction_layer,
                                                          Sip_SMCommand::dialog_layer) ) )
    {
        return false;
    }
    SRef<Sip_Request*> prack =dynamic_cast<Sip_Request *>(*command.get_command_packet());
    _dialog_state.update_state( prack );
    Sip_Uri peer(_dialog_state._remote_uri);
    string peerUri = peer.get_protocol_id() + ":" + peer.get_user_ip_string();
    if( !is_matching_prack( last_provisional, prack ) )
    {
        return false;
    }
    //      cerr << "RAck ok, send 200 Ok" << endl;
    last_provisional = NULL;
    // extract PRACK's content according to the RFC 3262 Reliability of Provisional Responses in SIP
    // since we sent a provisional responce with an offer the PRACK packet must contain the answer ( section 5)
    SRef < Sdp_Packet *> sdpPrack = (Sdp_Packet *) *prack->get_content();
    if ( !sdpPrack )
    {
        // send error message what to do ??
        send_cancel();
        error_mode = true;
        return false;
    }
    //	bool ret = sort_mime(*prack->get_content(), peerUri, 3);

    bool ret;
    if( (ret = get_media_session()->set_sdp_answer( (Sdp_Packet*)*prack->get_content(), peerUri )) )
    {
        cerr << "!!!!!!!!!!!!SDP answer rejected ??" << endl;
    }
    get_media_session()->set_last_sdp_answer ((Sdp_Packet*)*prack->get_content() );

    if ( ret == false )
    {
        // MIME NOT MATCH terminating our dialog
        //cerr<<"======================================= > failed to authorize the sdp answer carried in PRACK MIKEY error \n";
        send_cancel();
        error_mode = true;
        return false;
    }

    send_prack_ok( prack );
    send_ringing();

    Command_String cmdstr(_dialog_state._call_id,  Sip_Command_String::incoming_available,get_media_session()->get_peer_uri(),
                         (get_media_session()->is_secure()?"secure":"unprotected"));
    cmdstr["touri"]=prack->get_header_value_to()->get_uri().get_string();

    get_sip_stack()->get_callback()->handle_command("gui", cmdstr );
    return true;
}

bool Sip_Dialog_Voip_Server::a3008_100rel_ringing_PRACK( const Sip_SMCommand &command)
{
    if( ! (use100_rel &&
           last_provisional &&
           transition_match("PRACK", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer) ) )
    {
        return false;
    }

    SRef<Sip_Request*> prack =
            dynamic_cast<Sip_Request *>(*command.get_command_packet());
    _dialog_state.update_state( prack );

    if( !is_matching_prack( last_provisional, prack ) )
    {
        return false;
    }

    // 	cerr << "RAck ok, send 200 Ok" << endl;

    last_provisional = NULL;
    send_prack_ok( prack );

    Command_String cmdstr(_dialog_state._call_id,
                         Sip_Command_String::incoming_available,
                         get_media_session()->get_peer_uri(),
                         (get_media_session()->is_secure()?"secure":"unprotected")
                         );

    cmdstr["touri"]=prack->get_header_value_to()->get_uri().get_string();
    get_sip_stack()->get_callback()->handle_command("gui", cmdstr );

    send_ringing();

    return true;
}

bool Sip_Dialog_Voip_Server::a3009_any_any_ResendTimer1xx( const Sip_SMCommand &command)
{
    if( !transition_match(command, "ResendTimer1xx", Sip_SMCommand::dialog_layer, Sip_SMCommand::dialog_layer) )
    {
        return false;
    }

    if( !last_provisional )
    {
        // Stop retransmissions.
        return true;
    }

    resend_timer1xx *=2;

    // Stop retransmissions after 64*T1
    if( resend_timer1xx >= 64 * get_sip_stack()->get_timers()->getA() )
    {
        SRef<Sip_Response*> reject = create_sip_response( get_last_invite(), 504, "Server Time-out" );
        send_sip_message( *reject );
        Command_String cmdstr(_dialog_state._call_id, Sip_Command_String::remote_cancelled_invite,"");
        get_sip_stack()->get_callback()->handle_command("gui", cmdstr );

        get_media_session()->stop();
        signal_if_no_transactions();
        return true;
    }

    send_sip_message( last_provisional );
    request_timeout(resend_timer1xx,"ResendTimer1xx");
    return true;
}

bool Sip_Dialog_Voip_Server::a3010_any_any_PRACK( const Sip_SMCommand &command)
{
    if( ! (use100_rel &&
           transition_match("PRACK", command, Sip_SMCommand::transaction_layer, Sip_SMCommand::dialog_layer) ) )
    {
        return false;
    }

    SRef<Sip_Request*> prack = dynamic_cast<Sip_Request *>(*command.get_command_packet());
    _dialog_state.update_state( prack );

    if( !is_matching_prack( last_provisional, prack ) )
    {
        return false;
    }

    // 	cerr << "RAck ok, send 200 Ok" << endl;
    last_provisional = NULL;
    send_prack_ok( prack );

    return true;
}

void Sip_Dialog_Voip_Server::set_up_state_machine()
{
    State<Sip_SMCommand,string> *s_start=new State<Sip_SMCommand,string>(this,"start");
    add_state(s_start);

    State<Sip_SMCommand,string> *s_100rel=new State<Sip_SMCommand,string>(this,"100rel");
    add_state(s_100rel);

    State<Sip_SMCommand,string> *s_ringing=new State<Sip_SMCommand,string>(this,"ringing");
    add_state(s_ringing);

    State<Sip_SMCommand,string> *s_waitingACK=new State<Sip_SMCommand,string>(this,"waitingACK");
    add_state(s_waitingACK);

    State<Sip_SMCommand,string> *s_emptyInvite=new State<Sip_SMCommand,string>(this,"emptyInvite");
    add_state(s_emptyInvite);

    State<Sip_SMCommand,string> *s_100relNoOffer=new State<Sip_SMCommand,string>(this,"100relNoOffer");
    add_state(s_100relNoOffer);



    SRef<State<Sip_SMCommand,string> *> s_incall = get_state("incall");
    SRef<State<Sip_SMCommand,string> *> s_termwait= get_state("termwait");
    SRef<State<Sip_SMCommand,string> *> s_any = any_state;


    new State_Transition<Sip_SMCommand,string>(this, "transition_start_100rel_INVITE",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3007_start_100rel_INVITE,
                                             s_start, s_100rel);

    // Fallback to unreliable provisinal responses
    new State_Transition<Sip_SMCommand,string>(this, "transition_start_ringing_INVITE",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3001_start_ringing_INVITE,
                                             s_start, s_ringing);

    // adding for the SIP signaling flow when adding more media streams fromother clients
    new State_Transition<Sip_SMCommand,string>(this, "transition_start_100relNoOffer_INVITENOFFER",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a30077_start_100relNoOffer_INVITENOFFER,
                                             s_start,s_100relNoOffer);

    new State_Transition<Sip_SMCommand,string>(this, "transition_start_emptyInvite_emptyInvite",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3011_start_emptyInvite_emptyInvite,
                                             s_start, s_emptyInvite);


    // emptyInvite
    new State_Transition<Sip_SMCommand,string>(this, "transition_emptyInvite_waitingACK_USERACCEPTED",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3013_emptyInvite_waitingACK_USERACCEPTED,
                                             s_emptyInvite, s_waitingACK);


    new State_Transition<Sip_SMCommand,string>(this, "transition_emptyInvite_termwait_reject",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3005_ringing_termwait_reject,
                                             s_emptyInvite, s_termwait);

    new State_Transition<Sip_SMCommand,string>(this, "transition_emptyInvite_CANCEL",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3004_ringing_termwait_CANCEL,
                                             s_emptyInvite, s_termwait);


    ///////
    new State_Transition<Sip_SMCommand,string>(this, "transition_waitingACK_incall_arrivedACK",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3014_waitingACK_incall_arrivedACK,
                                             s_waitingACK, s_incall);

    new State_Transition<Sip_SMCommand,string>(this, "transition_waitingACK_termwait_BYE",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3015_waitingACK_termwait_BYE,
                                             s_waitingACK, s_termwait);


    new State_Transition<Sip_SMCommand,string>(this, "transition_a3004_ringing_termwait_CANCEL",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3004_ringing_termwait_CANCEL,
                                             s_waitingACK, s_termwait);



    //error in Mikey in waiting ACK

    new State_Transition<Sip_SMCommand,string>(this, "transition_a3111_any_termwait_error",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3111_any_termwait_error,
                                             s_waitingACK, s_termwait);


    //////////

    new State_Transition<Sip_SMCommand,string>(this, "transition_ringing_incall_accept",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3002_ringing_incall_accept,
                                             s_ringing, s_incall);

    new State_Transition<Sip_SMCommand,string>(this, "transition_incall_termwait_BYE",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3003_ringing_termwait_BYE,
                                             s_ringing, s_termwait);

    new State_Transition<Sip_SMCommand,string>(this, "transition_ringing_termwait_CANCEL",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3004_ringing_termwait_CANCEL,
                                             s_ringing, s_termwait);

    new State_Transition<Sip_SMCommand,string>(this, "transition_ringing_termwait_reject",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3005_ringing_termwait_reject,
                                             s_ringing, s_termwait);

    new State_Transition<Sip_SMCommand,string>(this, "transition_start_termwait_INVITEnothandled",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3006_start_termwait_INVITE,
                                             s_start, s_termwait);

    new State_Transition<Sip_SMCommand,string>(this, "transition_100rel_ringing_PRACK",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3008_100rel_ringing_PRACK,
                                             s_100rel, s_ringing);

    new State_Transition<Sip_SMCommand,string>(this, "transition_100rel_100rel_ResendTimer1xx",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3009_any_any_ResendTimer1xx,
                                             s_any, s_any);

    new State_Transition<Sip_SMCommand,string>(this, "transition_ringing_ringing_PRACK",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3010_any_any_PRACK,
                                             s_any, s_any);

    // 100rel -> termwait
    new State_Transition<Sip_SMCommand,string>(this, "transition_100rel_termwait_CANCEL",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3004_ringing_termwait_CANCEL,
                                             s_100rel, s_termwait);

    new State_Transition<Sip_SMCommand,string>(this, "transition_100rel_termwait_reject",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3005_ringing_termwait_reject,
                                             s_100rel, s_termwait);


    // registering states in s_100relNoOffer
    new State_Transition<Sip_SMCommand,string>(this, "transition_100relNoOffer_emptyInvite_PRACK",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a30088_100relNoOffer_emptyInvite_PRACK,
                                             s_100relNoOffer, s_emptyInvite);

    new State_Transition<Sip_SMCommand,string>(this, "transition_a3004_ringing_termwait_CANCEL",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3004_ringing_termwait_CANCEL,
                                             s_100relNoOffer, s_termwait);

    new State_Transition<Sip_SMCommand,string>(this, "transition_waitingACK_termwait_BYE",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3015_waitingACK_termwait_BYE,
                                             s_100relNoOffer, s_termwait);


    new State_Transition<Sip_SMCommand,string>(this, "transition_a3111_any_termwait_error",
                                             (bool (State_Machine<Sip_SMCommand,string>::*)(const Sip_SMCommand&)) &Sip_Dialog_Voip_Server::a3111_any_termwait_error,
                                             s_100relNoOffer, s_termwait);

    set_current_state(s_start);
}

Sip_Dialog_Voip_Server::Sip_Dialog_Voip_Server(SRef<Sip_Stack*> stack, SRef<Sip_Identity*> ident,
                                               bool stun, SRef<Session *> s, std::string cid)
    : Sip_Dialog_Voip(stack, ident, /*pconf*/ stun, s, cid),
      use100_rel( false ), resend_timer1xx( 0 )
{
    set_up_state_machine();
    no_offer = false;
    sdp_no_offer = NULL;
    checked_in_ringing = false;
    error_mode = false;
}

Sip_Dialog_Voip_Server::~Sip_Dialog_Voip_Server()
{
    media_session->unregister();
}

void Sip_Dialog_Voip_Server::send_invite_ok()
{
    SRef<Sip_Response*> ok= new Sip_Response(200,"OK", get_last_invite() );
    ok->get_header_value_to()->set_parameter("tag",_dialog_state._local_tag);

    SRef<Sip_Header_Value *> contact =
            new Sip_Header_Value_Contact( get_dialog_config()->get_contact_uri(use_stun), -1); //set expires to -1, we do not use it (only in register)
    ok->add_header( new Sip_Header(*contact) );

    if( !use100_rel )
    {
        //There might be so that there are no SDP. Check!
        SRef<Sdp_Packet *> sdp;
        if (media_session){
#ifdef ENABLE_TS
            ts.save("get_sdp_answer");
#endif
            sdp = media_session->get_sdp_answer();
#ifdef ENABLE_TS
            ts.save("get_sdp_answer");
#endif
            if( !sdp ){
                // FIXME: this most probably means that the
                // creation of the MIKEY message failed, it
                // should not happen
                my_err << "Sdp was NULL in send_invite_ok" << endl;
                return;
            }
        }

        /* Add the latter to the INVITE message */ // If it exists


        //-------------------------------------------------------------------------------------------------------------//
        ok->set_content( *sdp );
        //-------------------------------------------------------------------------------------------------------------//
        //	/* Get the SDP Answer from the MediaSession */
        //	SRef<Sdp_Packet *> sdpAnswer = media_session->get_sdp_answer();
        //
        //	if( sdpAnswer ){
        //		ok->set_content( *sdpAnswer );
        //	}
        //	/* if sdp is NULL, the offer was refused, send 606 */
        //	// FIXME
        //	else return;
    }

    SRef<Sip_Message*> pref(*ok);
    Sip_SMCommand cmd( pref, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE );
}

void Sip_Dialog_Voip_Server::send_reject()
{
    SRef<Sip_Response*> ringing = new Sip_Response(486,"Temporary unavailable", get_last_invite());
    ringing->get_header_value_to()->set_parameter("tag",_dialog_state._local_tag);
    SRef<Sip_Message*> pref(*ringing);
    Sip_SMCommand cmd( pref,Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE );
}

void Sip_Dialog_Voip_Server::send_ringing()
{
    SRef<Sip_Response*> ringing =
            create_sip_response( get_last_invite(), 180, "Ringing" );
    if (use100_rel)
    {
        ringing->add_header(new Sip_Header(new Sip_Header_Value_Require("100rel")));
        _dialog_state._rseq_no++;
        ringing->add_header(new Sip_Header(new Sip_Header_Value_RSeq( _dialog_state._rseq_no )));
        last_provisional = *ringing;
    }
    SRef<Sip_Header_Value *> contact =
            new Sip_Header_Value_Contact( get_dialog_config()->get_contact_uri(use_stun), -1); //set expires to -1, we do not use it (only in register)
    ringing->add_header( new Sip_Header(*contact) );
    send_sip_message( *ringing );
}

void Sip_Dialog_Voip_Server::send_ringing(SRef <Sdp_Packet *> sdpP)
{
    SRef<Sip_Response*> ringing =
            create_sip_response( get_last_invite(), 180, "Ringing" );
    if (use100_rel)
    {
        ringing->add_header(new Sip_Header(new Sip_Header_Value_Require("100rel")));
        _dialog_state._rseq_no++;
        ringing->add_header(new Sip_Header(new Sip_Header_Value_RSeq( _dialog_state._rseq_no )));
        last_provisional = *ringing;
    }
    SRef<Sip_Header_Value *> contact =
            new Sip_Header_Value_Contact(
                get_dialog_config()->get_contact_uri(use_stun),
                -1); //set expires to -1, we do not use it (only in register)
    ringing->add_header( new Sip_Header(*contact) );
    ringing->set_content( *sdpP );
    send_sip_message( *ringing );
}

void Sip_Dialog_Voip_Server::send_user_not_found()
{
    SRef<Sip_Response*> notFound = new Sip_Response(404,"Not found", get_last_invite());
    notFound->get_header_value_to()->set_parameter("tag",_dialog_state._local_tag);
    SRef<Sip_Message*> pref(*notFound);
    Sip_SMCommand cmd( pref, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE );
}

void Sip_Dialog_Voip_Server::send_not_acceptable()
{
    SRef<Sip_Response*> not_acceptable = new Sip_Response(406,"Not Acceptable", get_last_invite());
    if( media_session && media_session->get_error_string() != "" )
    {
        not_acceptable->add_header(
                    new Sip_Header(
                        new Sip_Header_Value_Warning(
                            get_sip_stack()->get_stack_config()->external_contact_ip,
                            399,
                            media_session->get_error_string() ) ));
    }

    not_acceptable->get_header_value_to()->set_parameter("tag",_dialog_state._local_tag);
    SRef<Sip_Message*> pref(*not_acceptable);
    Sip_SMCommand cmd( pref, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE );
}

void Sip_Dialog_Voip_Server::send_prack_ok( SRef<Sip_Request*> prack )
{
    SRef<Sip_Response*> ok = create_sip_response( prack, 200, "OK" );
    SRef<Sip_Header_Value *> contact =
            new Sip_Header_Value_Contact( get_dialog_config()->get_contact_uri(use_stun), -1); //set expires to -1, we do not use it (only in register)
    ok->add_header( new Sip_Header(*contact) );

    send_sip_message( *ok );
}

void Sip_Dialog_Voip_Server::send_session_progress()
{
    SRef<Sip_Response*> progress = create_sip_response( get_last_invite(), 183,"Session progress" );

    progress->add_header(new Sip_Header(new Sip_Header_Value_Require("100rel")));

    _dialog_state._rseq_no++;
    progress->add_header(new Sip_Header(new Sip_Header_Value_RSeq( _dialog_state._rseq_no )));

    SRef<Sip_Header_Value *> contact =
            new Sip_Header_Value_Contact( get_dialog_config()->get_contact_uri(use_stun), -1); //set expires to -1, we do not use it (only in register)
    progress->add_header( new Sip_Header(*contact) );

    SRef<Sdp_Packet *> sdp;
    if (media_session)
    {
#ifdef ENABLE_TS
        ts.save("get_sdp_answer");
#endif
        sdp = media_session->get_sdp_answer();
#ifdef ENABLE_TS
        ts.save("get_sdp_answer");
#endif
        if( !sdp ){
            // FIXME: this most probably means that the
            // creation of the MIKEY message failed, it
            // should not happen
            my_err << "Sdp was NULL in send_invite_ok" << endl;
            return;
        }
    }

    /* Add the latter to the INVITE message */ // If it exists
    progress->set_content( *sdp );

    last_provisional = *progress;

    send_sip_message( last_provisional );
}

void Sip_Dialog_Voip_Server::send_session_progress(SRef <Sdp_Packet*> sdp)
{
    SRef<Sip_Response*> progress =create_sip_response( get_last_invite(), 183,"Session progress" );
    progress->add_header(new Sip_Header(new Sip_Header_Value_Require("100rel")));
    _dialog_state._rseq_no++;
    progress->add_header(new Sip_Header(new Sip_Header_Value_RSeq( _dialog_state._rseq_no )));
    SRef<Sip_Header_Value *> contact = new Sip_Header_Value_Contact( get_dialog_config()->get_contact_uri(use_stun),-1); //set expires to -1, we do not use it (only in register)
    progress->add_header( new Sip_Header(*contact) );
    /* Add the latter to the INVITE message */ // If it exists
    progress->set_content( *sdp );
    last_provisional = *progress;
    send_sip_message( last_provisional );
}

void Sip_Dialog_Voip_Server::send_invite_ok(SRef <Sdp_Packet * > sdpPacket )
{
    SRef<Sip_Response*> ok= new Sip_Response(200,"OK", get_last_invite() );
    ok->get_header_value_to()->set_parameter("tag",_dialog_state._local_tag);

    SRef<Sip_Header_Value *> contact =
            new Sip_Header_Value_Contact( get_dialog_config()->get_contact_uri(use_stun), -1); //set expires to -1, we do not use it (only in register)
    ok->add_header( new Sip_Header(*contact) );

    ok->set_content( *sdpPacket );

    SRef<Sip_Message*> pref(*ok);
    Sip_SMCommand cmd( pref, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer);
    get_sip_stack()->enqueue_command(cmd, HIGH_PRIO_QUEUE );
}
