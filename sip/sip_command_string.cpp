#include "sip_command_string.h"

const std::string Sip_Command_String::sip_stack_shutdown="sip_stack_shutdown";
const std::string Sip_Command_String::sip_stack_shutdown_done="sip_stack_shutdown_done";
const std::string Sip_Command_String::no_op="no_op";
const std::string Sip_Command_String::register_all_identities="register_all_identities";
const std::string Sip_Command_String::register_all_identities_done="register_all_identities_done";
const std::string Sip_Command_String::unregister_all_identities="unregister_all_identities";
const std::string Sip_Command_String::unregister_all_identities_done="unregister_all_identities_done";
const std::string Sip_Command_String::terminate_all_calls="terminate_all_calls";
const std::string Sip_Command_String::terminate_all_calls_done="terminate_all_calls_done";

const std::string Sip_Command_String::transaction_terminated="transaction_terminated";
const std::string Sip_Command_String::call_terminated="call_terminated";
const std::string Sip_Command_String::call_terminated_early="call_terminated_early";
const std::string Sip_Command_String::no_transactions="no_transactions";

const std::string Sip_Command_String::error_message="error_message";

const std::string Sip_Command_String::authentication_failed="authentication_failed";
const std::string Sip_Command_String::transport_error="transport_error";

const std::string Sip_Command_String::hang_up="hang_up";
const std::string Sip_Command_String::invite="invite";
const std::string Sip_Command_String::invite_ok="invite_ok";
const std::string Sip_Command_String::invite_no_reply="invite_no_reply";
const std::string Sip_Command_String::incoming_available="incoming_available";
const std::string Sip_Command_String::remote_hang_up="remote_hang_up";
const std::string Sip_Command_String::remote_ringing="remote_ringing";
const std::string Sip_Command_String::remote_reject="remote_reject";
const std::string Sip_Command_String::remote_unacceptable="remote_unacceptable";
const std::string Sip_Command_String::cancel="cancel";
const std::string Sip_Command_String::cancel_ok="cancel_ok";
const std::string Sip_Command_String::remote_user_not_found="remote_user_not_found";
const std::string Sip_Command_String::accept_invite="accept_invite";
const std::string Sip_Command_String::reject_invite="reject_invite";
const std::string Sip_Command_String::remote_cancelled_invite="remote_cancelled_invite";
const std::string Sip_Command_String::accept_insecure="accept_insecure";
const std::string Sip_Command_String::reject_insecure="reject_insecure";
const std::string Sip_Command_String::security_failed="security_failed";

const std::string Sip_Command_String::proxy_register="proxy_register";
const std::string Sip_Command_String::register_sent="register_sent";
const std::string Sip_Command_String::register_no_reply="register_no_reply";
const std::string Sip_Command_String::register_ok="register_ok";
const std::string Sip_Command_String::register_failed="register_failed";
const std::string Sip_Command_String::register_failed_authentication="register_failed_authentication";
const std::string Sip_Command_String::temp_unavail="temp_unavail";

const std::string Sip_Command_String::close_window="close_window";
const std::string Sip_Command_String::ask_password="ask_password";
const std::string Sip_Command_String::setpassword="setpassword";
const std::string Sip_Command_String::incoming_im="incoming_im";
const std::string Sip_Command_String::outgoing_im="outgoing_im";

const std::string Sip_Command_String::start_presence_client="start_presence_client";
const std::string Sip_Command_String::stop_presence_client="stop_presence_client";
const std::string Sip_Command_String::remote_presence_update="remote_presence_update";

const std::string Sip_Command_String::start_presence_server= "start_presence_server";
const std::string Sip_Command_String::stop_presence_server= "stop_presence_server";
const std::string Sip_Command_String::local_presence_update= "local_presence_update";

/* Transfer initiation */
const std::string Sip_Command_String::user_transfer = "user_transfer";
const std::string Sip_Command_String::transfer_pending = "transfer_pending";
const std::string Sip_Command_String::transfer_refused = "transfer_refused";
const std::string Sip_Command_String::transfer_failed = "transfer_failed";

/* Transfer answering */
const std::string Sip_Command_String::transfer_requested = "transfer_requested";
const std::string Sip_Command_String::user_transfer_accept = "user_transfer_accept";
const std::string Sip_Command_String::user_transfer_refuse = "user_transfer_refuse";
const std::string Sip_Command_String::call_transferred = "call_transferred";


/******** Add Participant **********/

const std::string Sip_Command_String::participant_request="participant_request";
const std::string Sip_Command_String::participant_answer_sdp_offer="participant_answer_sdp_offer";
const std::string Sip_Command_String::participant_reinvite_answer="participant_reinvite_answer";
const std::string Sip_Command_String::participant_done="participant_done";
const std::string Sip_Command_String::participant_bye="participant_bye";
const std::string Sip_Command_String::participant_failure="participant_failure";
const std::string Sip_Command_String::participant_finally_added = "participant_finally_added";
