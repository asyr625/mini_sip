#ifndef SIP_COMMAND_DISPATCHER_H
#define SIP_COMMAND_DISPATCHER_H

#include "sobject.h"
#include "message_router.h"
#include "mini_list.h"
#include "my_semaphore.h"

#include "sip_smcommand.h"
#include "sip_dialog.h"
#include "sip_stack.h"
#include "sip_transaction.h"

#include "sip_layer_dialog.h"
#include "sip_layer_transaction.h"

typedef struct queue_type
{
    int type;
    SRef<Sip_SMCommand*> command;
    SRef<Sip_Transaction*> transaction_receiver;
    SRef<Sip_Dialog*> call_receiver;
} queue_type;

class Sip_Layer_Dialog;
class Sip_Layer_Transport;
class Sip_Stack_Internal;

class Sip_Layer_Transaction;

#define TYPE_COMMAND 2
#define TYPE_TIMEOUT 3

class Sip_Command_Dispatcher : public SObject
{
public:
    Sip_Command_Dispatcher(SRef<Sip_Stack_Internal*> stack, SRef<Sip_Layer_Transport*> transport);

    void free();

    void set_callback(SRef<Command_Receiver*> cb);
    SRef<Command_Receiver*> get_callback();

    void add_dialog(SRef<Sip_Dialog*> d);
    std::list<SRef<Sip_Dialog *> > get_dialogs();

    void set_dialog_management(SRef<Sip_Dialog*> mgmt);

    virtual void run();
    void stop_running();

    SRef<Sip_Stack_Internal*> get_sip_stack_internal();

    virtual std::string get_mem_object_type() const {return "SipCommandDispatcher";}

    virtual bool handle_command(const Sip_SMCommand &c);
    bool dispatch(const Sip_SMCommand &cmd);

    bool maintainence_handle_command(const Sip_SMCommand &c);

    void enqueue_command(const Sip_SMCommand &cmd, int queue = LOW_PRIO_QUEUE);

    void enqueue_timeout(SRef<Sip_Transaction*> receiver, const Sip_SMCommand &command);
    void enqueue_timeout(SRef<Sip_Dialog*> receiver, const Sip_SMCommand &command);


    SRef<Sip_Layer_Transport*> get_layer_transport();
    SRef<Sip_Layer_Transaction*> get_layer_transaction();
    SRef<Sip_Layer_Dialog*> get_layer_dialog();

    SRef<Sip_Dialog*>  get_dialog(std::string call_id);

    SRef<Sip_Dialog*> _management_handler;

    void set_inform_transaction_terminate(bool do_inform){ _inform_tu_on_transaction_terminate = do_inform; }

private:
    SRef<Command_Receiver*> _callback;
    SRef<Sip_Stack_Internal*> _sip_stack_internal;

    Semaphore _semaphore;
    Mutex _mlock;
    Mini_List<queue_type> _high_prio_command_q;
    Mini_List<queue_type> _low_prio_command_q;

    SRef<Sip_Layer_Dialog*> _dialog_layer;

    SRef<Sip_Layer_Transaction*> _transaction_layer;

    SRef<Sip_Layer_Transport*> _transport_layer;

    Mutex _dialog_list_lock;

    bool _keep_running;

    bool _inform_tu_on_transaction_terminate;
};

#endif // SIP_COMMAND_DISPATCHER_H
