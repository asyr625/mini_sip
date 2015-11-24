#include <iostream>
#include "sip_command_dispatcher.h"
#include "sip_layer_dialog.h"
#include "sip_layer_transaction.h"
#include "sip_layer_transport.h"

#include "sip_command_string.h"

Sip_Command_Dispatcher::Sip_Command_Dispatcher(SRef<Sip_Stack_Internal*> stack, SRef<Sip_Layer_Transport*> transport)
    :_sip_stack_internal(stack),
      _keep_running(true),
      _inform_tu_on_transaction_terminate(false)
{
    _transport_layer = transport;
    _transaction_layer = new Sip_Layer_Transaction(this,_transport_layer);

    _dialog_layer = new Sip_Layer_Dialog(this);
    _transport_layer->set_dispatcher(this);
}

void Sip_Command_Dispatcher::free()
{
    _sip_stack_internal = NULL;
    _callback = NULL;
    if ( _management_handler ){
        _management_handler->free_state_machine();
        _management_handler = NULL;
    }
    _transport_layer->set_dispatcher(NULL);
    _transport_layer = NULL;
    _dialog_layer = NULL;
    _transaction_layer = NULL;
}

void Sip_Command_Dispatcher::set_callback(SRef<Command_Receiver*> cb)
{
    _callback = cb;
}

SRef<Command_Receiver*> Sip_Command_Dispatcher::get_callback()
{
    return _callback;
}

void Sip_Command_Dispatcher::add_dialog(SRef<Sip_Dialog*> d)
{
    _dialog_layer->add_dialog(d);
}

std::list<SRef<Sip_Dialog *> > Sip_Command_Dispatcher::get_dialogs()
{
    _dialog_layer->get_dialogs();
}

void Sip_Command_Dispatcher::set_dialog_management(SRef<Sip_Dialog*> mgmt)
{
    _management_handler = mgmt;
}

void Sip_Command_Dispatcher::stop_running()
{
    _keep_running = false;
    _transport_layer->stop();

    Command_String c("", Sip_Command_String::no_op);
    Sip_SMCommand sc(c, Sip_SMCommand::dispatcher, Sip_SMCommand::dispatcher);
    enqueue_command(sc, LOW_PRIO_QUEUE);
}


SRef<Sip_Layer_Transport*> Sip_Command_Dispatcher::get_layer_transport()
{
    return _transport_layer;
}

SRef<Sip_Layer_Transaction*> Sip_Command_Dispatcher::get_layer_transaction()
{
    return _transaction_layer;
}

SRef<Sip_Layer_Dialog*> Sip_Command_Dispatcher::get_layer_dialog()
{
    return _dialog_layer;
}

SRef<Sip_Dialog*> Sip_Command_Dispatcher::get_dialog(std::string call_id)
{
    return _dialog_layer->get_dialog(call_id);
}

void Sip_Command_Dispatcher::run()
{
    while( _keep_running )
    {
        my_dbg("signaling/sip") << "DIALOG CONTAINER: waiting for command"<< std::endl;
        _semaphore.dec();

        struct queue_type item;

        _mlock.lock();
        if( _high_prio_command_q.size() > 0 )
            item = _high_prio_command_q.pop_back();
        else
        {
            my_assert( _low_prio_command_q.size() > 0 );
            item = _low_prio_command_q.pop_back();
        }
        _mlock.unlock();

#ifdef DEBUG_OUTPUT
        if (item.type==TYPE_COMMAND)
            my_dbg("signaling/sip") << "DISPATCHER: got command: "<< **item.command << std::endl;
        else
            my_dbg("signaling/sip") << "DISPATCHER: got timeout: "<< **item.command << std::endl;
#endif

        bool handled = false;
        if( item.type == TYPE_COMMAND )
        {
            handled = handle_command(**(item.command));
        }
        else
        {    // item.type == TYPE_TIMEOUT
            if( !item.transaction_receiver.is_null() )
            {
                handled = item.transaction_receiver->handle_command(**item.command);
            }

            if ( !item.call_receiver.is_null() )
            {
                handled = item.call_receiver->handle_command(**item.command);
            }
        }

#ifdef DEBUG_OUTPUT
        if( !handled )
        {
            my_err << "DISPATCHER: command NOT handled:" << **(item.command) << std::endl;
        }
#endif

        item.command = NULL;
        item.transaction_receiver = NULL;
        item.call_receiver = NULL;
    }
}



SRef<Sip_Stack_Internal*> Sip_Command_Dispatcher::get_sip_stack_internal()
{
    return _sip_stack_internal;
}


void Sip_Command_Dispatcher::enqueue_command(const Sip_SMCommand &command, int queue)
{
#ifdef DEBUG_OUTPUT
    my_dbg("signaling/sip") << "Dispatcher: enqueue(" << command << ")" << std::endl;
#endif
    struct queue_type item;
    item.type = TYPE_COMMAND;
    item.command = SRef<Sip_SMCommand*>(new Sip_SMCommand(command));

    _mlock.lock();
    if (queue == HIGH_PRIO_QUEUE)
    {
        _high_prio_command_q.push_front(item);
    }
    else
    {
        _low_prio_command_q.push_front(item);
    }
    _mlock.unlock();
    _semaphore.inc();
}

void Sip_Command_Dispatcher::enqueue_timeout(SRef<Sip_Transaction *> receiver, const Sip_SMCommand &command)
{
#ifdef DEBUG_OUTPUT
    my_dbg("signaling/sip") << "Dispatcher: enqueue(" << command << ")" << std::endl;
#endif

    struct queue_type item;
    item.type = TYPE_TIMEOUT;
    item.command = SRef<Sip_SMCommand*>( new Sip_SMCommand(command));
    item.transaction_receiver = receiver;
    item.call_receiver = NULL;

    _mlock.lock();
    _high_prio_command_q.push_front(item);
    _mlock.unlock();

    _semaphore.inc();
}

void Sip_Command_Dispatcher::enqueue_timeout(SRef<Sip_Dialog*> receiver, const Sip_SMCommand &command)
{
#ifdef DEBUG_OUTPUT
    my_dbg("signaling/sip") << "Dispatcher: enqueue(" << command << ")" << std::endl;
#endif

    struct queue_type item;
    item.type = TYPE_TIMEOUT;
    item.command = SRef<Sip_SMCommand*>( new Sip_SMCommand(command));
    item.transaction_receiver = NULL;
    item.call_receiver = receiver;

    _mlock.lock();
    _high_prio_command_q.push_front(item);
    _mlock.unlock();

    _semaphore.inc();
}

bool Sip_Command_Dispatcher::handle_command(const Sip_SMCommand &c)
{
#ifdef DEBUG_OUTPUT
    my_dbg("signaling/sip") << "DISPATCHER: SipCommandDispatcher got command "<< c<<std::endl;
#endif

    int dst = c.get_destination();

    bool ret = false;
    if( dst == Sip_SMCommand::dialog_layer )
    {
        if( c.get_source() != Sip_SMCommand::dialog_layer &&
                c.get_source()!=Sip_SMCommand::transaction_layer)
        {
            my_dbg("signaling/sip") << "DISPATCHER: WARNING: Dialog layer is expected to receive commands only from dialog or trasaction" << std::endl;
        }
        ret = _dialog_layer->handle_command(c);
    }
    else if( dst == Sip_SMCommand::transaction_layer )
    {
        ret = _transaction_layer->handle_command(c);
    }
    else if( dst == Sip_SMCommand::transport_layer )
    {
        if( c.get_source() != Sip_SMCommand::transaction_layer
                && c.get_source() != Sip_SMCommand::transaction_layer
                && !(c.get_type() == Sip_SMCommand::COMMAND_PACKET
                     && c.get_command_packet()->get_type()=="ACK"))
        {
            my_dbg("signaling/sip") << "DISPATCHER: WARNING: Transport layer is expected to receive commands only from trasaction" << std::endl;
        }
        ret = _transport_layer->handle_command(c);
    }
    else if( dst == Sip_SMCommand::dispatcher )
    {
        ret = maintainence_handle_command(c);
    }
    else
    {
        std::cerr << "ERROR: SipCommandDispatcher::handleCommand: Unknown destination (layer)" << std::endl;
    }

    if( !ret && c.get_source() == Sip_SMCommand::transport_layer )
    {
        Sip_SMCommand cmd( c );
        cmd.set_destination( Sip_SMCommand::dialog_layer );
        ret = _dialog_layer->handle_command( cmd );
    }

    if( !ret )
    {
#ifdef DEBUG_OUTPUT
        my_dbg("signaling/sip") <<"WARNING SipCommandDispatcher: The destination layer did not handle the command!"<<std::endl;
#endif
    }
    return ret;
}


bool Sip_Command_Dispatcher::maintainence_handle_command(const Sip_SMCommand &c)
{
    if( c.get_type() == Sip_SMCommand::COMMAND_STRING )
    {
        if( c.get_command_string().get_op() == Sip_Command_String::no_op )
        {
            return true;
        }

        if( c.get_command_string().get_op()==Sip_Command_String::transaction_terminated )
        {
            std::string tid = c.get_command_string().get_destination_id();
            SRef<Sip_Transaction *> t = _transaction_layer->get_transaction(tid);
            if (!t)
            {
                my_err<<"ERROR: SipCommandDispatcher::maintainenceHandleCommand: could not handle transaction_terminated for id " << tid << std::endl;
                return false;
            }
            std::string cid = t->get_call_id();
            t = NULL;
            _transaction_layer->remove_transaction(tid);

            if (_inform_tu_on_transaction_terminate)
            {
                Sip_SMCommand tterm(
                            Command_String( cid, Sip_Command_String::transaction_terminated, tid ),
                            Sip_SMCommand::transaction_layer,
                            Sip_SMCommand::dialog_layer
                            );
                enqueue_command(tterm, HIGH_PRIO_QUEUE);
            }

            SRef<Sip_Dialog*> d = _dialog_layer->get_dialog(cid);
            //It is ok to not find a dialog (transaction
            //without dialog).
            if( d )
            {
                d->signal_if_no_transactions();
            }
            return true;
        }
        else if( c.get_command_string().get_op() == Sip_Command_String::call_terminated )
        {
            return _dialog_layer->remove_dialog(c.get_destination_id());
        }
        else if(  _management_handler && (c.get_command_string().get_op() == Sip_Command_String::sip_stack_shutdown ||
                                          c.get_command_string().get_op() == Sip_Command_String::register_all_identities ||
                                          c.get_command_string().get_op() == Sip_Command_String::register_all_identities_done ||
                                          c.get_command_string().get_op() == Sip_Command_String::unregister_all_identities ||
                                          c.get_command_string().get_op() == Sip_Command_String::unregister_all_identities_done ||
                                          c.get_command_string().get_op() == Sip_Command_String::terminate_all_calls ||
                                          c.get_command_string().get_op() == Sip_Command_String::terminate_all_calls_done ||
                                          c.get_command_string().get_op() == Sip_Command_String::call_terminated_early ||
                                          c.get_command_string().get_op() == Sip_Command_String::register_ok ) )
        {
            //commands that are only interesting to the management dialog ...
            //Refurbish the command ... or the SipDialog::handleCmd won't let it through
            if( _management_handler )
            {
                Sip_SMCommand cmd( c.get_command_string(),
                                   Sip_SMCommand::dispatcher,
                                   Sip_SMCommand::dialog_layer);	//It's a SipDialog sub-class
                _management_handler->handle_command(cmd);
            }
            return true;

        }
        else if( _management_handler && c.get_command_string().get_op() == Sip_Command_String::sip_stack_shutdown_done )
        {

            //			Sip_SMCommand cmd( c.get_command_string(),
            //					Sip_SMCommand::dispatcher,
            //					Sip_SMCommand::dispatcher);
            // 			managementHandler->handleCommand(cmd); //process the command, so it moves to terminated state
            _management_handler->get_sip_stack()->stop_running();
            return true;
        }
        else
        {
#ifdef DEBUG_OUTPUT
            my_dbg("signaling/sip") << "SipCommandDispatcher: Error: maintainenceHandleCommand did not understand command: "<< c << std::endl;
#endif
            return false;
        }
    }
    return false;
}


