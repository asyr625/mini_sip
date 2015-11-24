#ifndef SIP_DIALOG_MANAGEMENT_H
#define SIP_DIALOG_MANAGEMENT_H

#include "sip_dialog.h"
#include "state_machine.h"

class Sip_Dialog_Management : public Sip_Dialog
{
public:
    Sip_Dialog_Management(SRef<Sip_Stack*> stack);

    virtual ~Sip_Dialog_Management();

    virtual std::string get_mem_object_type() const {return "SipDialogManagement";}

    virtual std::string get_name(){return "SipDialogManagement (The one and only)";}

private:
    void set_up_state_machine();
    void set_up_state_machine_shutdown(State<Sip_SMCommand,std::string> *s_start);
    void set_up_state_machine_dialogops(State<Sip_SMCommand,std::string> *s_start);

    bool a0_start_startShutdown_startShutdown( const Sip_SMCommand &command);
    bool a10_startSh_terminateCallsSh_terminateAll( const Sip_SMCommand &command);
    bool a11_terminateCallsSh_callTerminatedEarly( const Sip_SMCommand &command);
    bool a12_terminateCallsSh_timeIsUp( const Sip_SMCommand &command);
    bool a13_terminateCallsSh_allTerminated( const Sip_SMCommand &command);
    bool a20_terminateCallsSh_deRegAllSh_allTerminated( const Sip_SMCommand &command);
    bool a21_deRegAllSh_callTerminatedEarly( const Sip_SMCommand &command);
    bool a22_deRegAllSh_registerOk( const Sip_SMCommand &command);
    bool a23_deRegAllSh_timeIsUp( const Sip_SMCommand &command);
    bool a24_deRegAllSh_deRegAlldone( const Sip_SMCommand &command);
    bool a25_deRegAllSh_allTerminated( const Sip_SMCommand &command);
    bool a30_deRegAllSh_terminated_timeIsUp( const Sip_SMCommand &command);

    bool b0_start_terminateCallsOps_terminateAll( const Sip_SMCommand &command);
    bool b11_terminateCallsOps_terminateEarly( const Sip_SMCommand &command);
    bool b12_terminateCallsOps_timeIsUp( const Sip_SMCommand &command);
    bool b30_terminateCallsOps_start_terminateAllDone( const Sip_SMCommand &command);

    bool c0_start_deRegAllOps_deRegAll( const Sip_SMCommand &command);
    bool c11_deRegAllOps_registerOk( const Sip_SMCommand &command);
    bool c12_deRegAllOps_timeIsUp( const Sip_SMCommand &command);
    bool c30_deRegAllOps_start_deRegAllDone( const Sip_SMCommand &command);


    bool d0_start_regAllOps_regAll( const Sip_SMCommand &command);
    bool d11_regAllOps_registerOk( const Sip_SMCommand &command);
    bool d12_regAllOps_timeIsUp( const Sip_SMCommand &command);
    bool d30_regAllOps_start_regAllDone( const Sip_SMCommand &command);

    bool terminate_all_calls();

    bool received_call_terminate_early();

    bool de_register_all();

    bool register_all();

    bool received_register_ok(bool deregistering);

    bool shutdown_done( bool force );

    int pending_hang_ups;

    int pending_de_regs;
};

#endif // SIP_DIALOG_MANAGEMENT_H
