#ifndef SIP_COMMAND_STRING_H
#define SIP_COMMAND_STRING_H

#include <string>

class Sip_Command_String
{
public:
    /*
    Stack management
    */
    static const std::string sip_stack_shutdown;
    static const std::string sip_stack_shutdown_done;
    static const std::string no_op;
    static const std::string register_all_identities;
    static const std::string register_all_identities_done;
    static const std::string unregister_all_identities;
    static const std::string unregister_all_identities_done;
    static const std::string terminate_all_calls;
    static const std::string terminate_all_calls_done;

    /*
    Transaction/Dialog related
    */
    static const std::string transaction_terminated;
    static const std::string call_terminated;
    static const std::string call_terminated_early;
    static const std::string no_transactions;

    /*
    Action triggers or result information
    */
    static const std::string error_message;
    static const std::string transport_error;
    static const std::string authentication_failed;
    static const std::string hang_up;
    static const std::string invite;
    static const std::string invite_ok;
    static const std::string invite_no_reply;
    static const std::string incoming_available;
    static const std::string remote_hang_up;
    static const std::string cancel;
    static const std::string cancel_ok;
    static const std::string remote_user_not_found;
    static const std::string accept_invite;
    static const std::string reject_invite;
    static const std::string remote_cancelled_invite;
    static const std::string remote_ringing;
    static const std::string remote_reject;
    static const std::string remote_unacceptable;
    static const std::string accept_insecure;
    static const std::string reject_insecure;
    static const std::string security_failed;
    static const std::string setpassword;

    /*
    Registration Related
    */
    static const std::string proxy_register;
    static const std::string register_sent;
    static const std::string register_no_reply;
    static const std::string register_ok;
    static const std::string register_failed;
    static const std::string register_failed_authentication;
    static const std::string temp_unavail;

    static const std::string close_window;
    static const std::string ask_password;

    static const std::string incoming_im;
    static const std::string outgoing_im;

    static const std::string start_presence_client;
    static const std::string stop_presence_client;
    static const std::string remote_presence_update;

    static const std::string start_presence_server;
    static const std::string stop_presence_server;
    static const std::string local_presence_update;

    /* Transfer initiation */
    static const std::string user_transfer;
    static const std::string transfer_pending;
    static const std::string transfer_refused;
    static const std::string transfer_failed;

    /* Transfer answering */
    static const std::string transfer_requested;
    static const std::string user_transfer_accept;
    static const std::string user_transfer_refuse;
    static const std::string call_transferred;

    /* add Participant */
    static const std :: string participant_request;
    static const std :: string participant_answer_sdp_offer;
    static const std :: string participant_reinvite_answer;
    static const std :: string participant_done;
    static const std :: string participant_bye;
    static const std :: string participant_hang_up;
    static const std :: string participant_failure;
    static const std :: string participant_finally_added;
};

#endif // SIP_COMMAND_STRING_H
