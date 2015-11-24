#ifndef SIP_DIALOG_VOIP_SERVER_H
#define SIP_DIALOG_VOIP_SERVER_H

#include "sip_stack.h"
#include "sip_response.h"
#include "sip_dialog_voip.h"
#include "sip_configuration.h"
#include "state_machine.h"

class Sip_Dialog_Voip_Server : public Sip_Dialog_Voip
{
public:
    Sip_Dialog_Voip_Server(SRef<Sip_Stack*> stack, SRef<Sip_Identity*> ident,
            bool stun, SRef<Session *> s, std::string cid="");

    virtual ~Sip_Dialog_Voip_Server();

    virtual std::string get_mem_object_type() const { return "SipDialogVoipServer"; }

    virtual std::string get_name() { return "SipDialogVoipServer[callid=" + _dialog_state._call_id +"]"; }

    std::string name ;

protected:
    void send_ringing();
    void send_ringing(SRef <Sdp_Packet *> sdpP);
    bool is_matching_prack( SRef<Sip_Message*> provisional, SRef<Sip_Request*> prack );

    SRef<Sdp_Packet *> sdp_no_offer;

private:

    bool no_offer;
    void set_up_state_machine();
    bool error_mode ;
    void send_invite_ok();
    void send_invite_ok(SRef <Sdp_Packet * > sdpPacket );

    bool checked_in_ringing;
    void send_reject();
    void send_user_not_found();
    void send_not_acceptable();
    void send_session_progress();
    void send_session_progress(SRef <Sdp_Packet*> sdp);
    void send_prack_ok( SRef<Sip_Request*> prack );

    bool a3001_start_ringing_INVITE( const Sip_SMCommand &command);
    bool a3002_ringing_incall_accept( const Sip_SMCommand &command);
    bool a3003_ringing_termwait_BYE( const Sip_SMCommand &command);
    bool a3004_ringing_termwait_CANCEL( const Sip_SMCommand &command);
    bool a3005_ringing_termwait_reject( const Sip_SMCommand &command);
    bool a3006_start_termwait_INVITE( const Sip_SMCommand &command);
    bool a3007_start_100rel_INVITE( const Sip_SMCommand &command);
    bool a3008_100rel_ringing_PRACK( const Sip_SMCommand &command);
    bool a3009_any_any_ResendTimer1xx( const Sip_SMCommand &command);
    bool a3010_any_any_PRACK( const Sip_SMCommand &command);

    // adding participant
    bool a3014_waitingACK_incall_arrivedACK(const Sip_SMCommand &command);
    bool a3013_emptyInvite_waitingACK_USERACCEPTED(const Sip_SMCommand &command);
    bool a3011_start_emptyInvite_emptyInvite(const Sip_SMCommand &command);
    bool a3015_waitingACK_termwait_BYE( const Sip_SMCommand &command);
    bool a30077_start_100relNoOffer_INVITENOFFER( const Sip_SMCommand &command);
    bool a30088_100relNoOffer_emptyInvite_PRACK( const Sip_SMCommand &command);
    bool a3111_any_termwait_error( const Sip_SMCommand &command);

    bool use100_rel;
    int resend_timer1xx;
    SRef<Sip_Message*> last_provisional;
    State<Sip_SMCommand, std::string> *s_waiting_acks;
};

#endif // SIP_DIALOG_VOIP_SERVER_H
