#ifndef SIP_DIALOG_VOIP_H
#define SIP_DIALOG_VOIP_H

#include "sip_dialog.h"
#include "sip_response.h"
#include "state_machine.h"

#include "session.h"
#include "irequest_video_keyframe.h"

class Log_Entry;

class Sip_Dialog_Voip : public Sip_Dialog, public IRequest_Video_Keyframe
{
public:
    Sip_Dialog_Voip(SRef<Sip_Stack*> stack,
                    SRef<Sip_Identity*> ident,
                    bool usestun,
                    SRef<Session *> mediasession,
                    std::string cid=""
                );
    virtual ~Sip_Dialog_Voip();

    virtual std::string get_mem_object_type() const {return "SipDialogVoip";}

    virtual std::string get_name(){return "SipDialogVoip[callid="+ _dialog_state._call_id +"]";}


    virtual bool handle_command(const Sip_SMCommand &command);
    virtual void send_ack();

    void set_media_session(SRef<Session*>s);
    SRef<Session *> get_media_session();


    void send_ack_sdp (SRef <Sdp_Packet *> p);
    std :: string get_call_id (){ return _dialog_state._call_id;}
    std :: string get_uri(){return _dialog_state._remote_uri;}

    virtual void try_requesting_video_keyframe();

protected:
    int source_stream_id;
    void send_bye(int);
    void send_bye_ok(SRef<Sip_Request*> bye);
    void send_cancel();
    SRef<Sip_Request*> get_last_invite();
    void set_last_invite(SRef<Sip_Request*> i);
    bool sort_mime(SRef<Sip_Message_Content *> Offer, std::string peerUri, int type);
    SRef<Log_Entry *> get_log_entry();
    void set_log_entry(SRef<Log_Entry *> l);

    void register_sdp(uint32_t sourceId, SRef<Sdp_Packet*> sdppack);

    std::vector <SRef <Sdp_Header * > > reinv_headers;
    std :: string pushing_rtp_traffic_flag;
    std :: string actionF;
    SRef<Session *> media_session;

    bool notify_early_termination;

    bool use_stun;

    void send_reinvite(SRef <Sdp_Packet *> sdpPacket );

    bool participant_forwarder_mode ;
    bool reflector_mode;

private:
    void set_up_state_machine();

    void send_refer_ok();
    void send_notify_ok(SRef<Sip_Request*> notif);
    void send_info_ok(SRef<Sip_Request*> info);
    void send_refer_reject();
    void send_refer(int, const std::string referredUri);

    bool a1011_incall_incall_REINVITE( const Sip_SMCommand &command);
    bool a1012_incall_incall_INFO(const Sip_SMCommand &command);
    bool a1019_ReInviteSent_Incall_200OK ( const Sip_SMCommand &command);

    bool a1001_incall_termwait_BYE( const Sip_SMCommand &command);
    bool a1002_incall_byerequest_hangup( const Sip_SMCommand &command);
    bool a1003_byerequest_termwait_26( const Sip_SMCommand &command);

    bool a1101_termwait_terminated_notransactions( const Sip_SMCommand &command);
    bool a1102_termwait_termwait_early( const Sip_SMCommand &command);

    bool a1201_incall_transferrequested_transfer( const Sip_SMCommand &command);
    bool a1202_transferrequested_transferpending_202( const Sip_SMCommand &command);
    bool a1203_transferrequested_incall_36( const Sip_SMCommand &command);
    bool a1204_transferpending_transferpending_notify( const Sip_SMCommand &command);

    bool a1301_incall_transferaskuser_REFER( const Sip_SMCommand &command);
    bool a1302_transferaskuser_transferstarted_accept( const Sip_SMCommand &command);
    bool a1303_transferaskuser_incall_refuse( const Sip_SMCommand &command);

    SRef<Log_Entry *> log_entry;

    SRef<Sip_Request*> last_invite;
    SRef<Sip_Request*> last_refer;


    bool first_time_included;
    int  line_of_addition ;
};

#endif // SIP_DIALOG_VOIP_H
