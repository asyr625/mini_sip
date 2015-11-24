#ifndef SIP_DIALOG_PRESENCE_SERVER_H
#define SIP_DIALOG_PRESENCE_SERVER_H
#include "mini_list.h"
#include "sip_dialog.h"

class Sip_Dialog_Presence_Server : public Sip_Dialog
{
public:
    Sip_Dialog_Presence_Server(SRef<Sip_Stack*> dContainer, SRef<Sip_Identity*> ident, bool use_stun);

    virtual ~Sip_Dialog_Presence_Server();

    virtual std::string get_mem_object_type() const {return "SipDialogPresenceServer";}

    virtual  std::string get_name(){return "SipDialogPresenceServer[callid="+ _dialog_state._call_id +"]";}

    virtual bool handle_command(const Sip_SMCommand &c);

    void set_up_state_machine();

private:
    void send_notice_to_all( std::string onlineStatus);
    void send_notice( std::string onlinestatus,  std::string user);
    void send_subscribe_ok(SRef<Sip_Request*> sub);
    void remove_user( std::string user);
    void add_user( std::string user);

    void send_notify(std::string toUri,  std::string cid);

    bool a0_start_default_startpresenceserver(const Sip_SMCommand &command);
    bool a1_default_default_timerremovesubscriber(const Sip_SMCommand &command);
    bool a2_default_default_localpresenceupdated(const Sip_SMCommand &command);
    bool a3_default_termwait_stoppresenceserver(const Sip_SMCommand &command);
    bool a4_termwait_terminated_notransactions(const Sip_SMCommand &command);
    bool a5_default_default_SUBSCRIBE(const Sip_SMCommand &command);

    bool _use_stun;
    Mini_List< std::string> _subscribing_users;
    Mutex _users_lock;

    std::string _online_status;
};

#endif // SIP_DIALOG_PRESENCE_SERVER_H
