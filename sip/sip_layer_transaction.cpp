#include <iostream>
#include "sip_layer_transaction.h"
#include "sip_transaction.h"
#include "sip_command_dispatcher.h"

Sip_Layer_Transaction::Sip_Layer_Transaction(SRef<Sip_Command_Dispatcher*> dispatcher,
        SRef<Sip_Layer_Transport*> transp)
    : _handle_ack(true),
      _dispatcher(dispatcher),
      _transport_layer(transp)
{
}

Sip_Layer_Transaction::~Sip_Layer_Transaction()
{
    std::map<std::string, SRef<Sip_Transaction*> >::iterator i;
    for(i = _transactions.begin(); i != _transactions.end(); ++i)
    {
        (*i).second->free_state_machine();
    }
}


std::string Sip_Layer_Transaction::create_client_transaction( SRef<Sip_Request*> req )
{
    SRef<Sip_Transaction*> new_transaction;
    new_transaction = Sip_Transaction::create(_dispatcher->get_sip_stack_internal(),
                                              req,
                                              true,
                                              _handle_ack);

    if( new_transaction )
    {
        add_transaction(new_transaction);
        bool handled = new_transaction->handle_command(Sip_SMCommand(*req, Sip_SMCommand::dialog_layer, Sip_SMCommand::transaction_layer));
        if (!handled)
        {
            std::cerr <<"ERROR: TransactionLayer::defaultCommandHandler: Transaction refused command: "<<req<<std::endl;
        }
        return new_transaction->get_transaction_id();
    }
    return "";
}

bool Sip_Layer_Transaction::default_command_handler(const Sip_SMCommand &cmd)
{
    SRef<Sip_Transaction*> new_transaction;

    if(cmd.get_type() == Sip_SMCommand::COMMAND_PACKET && cmd.get_command_packet()->get_type() != Sip_Response::type)
    {
        SRef<Sip_Request*> req = dynamic_cast<Sip_Request*>(*cmd.get_command_packet());
        my_assert(req);

        if( !_handle_ack && req->get_method() == "ACK" )
        {
            new_transaction = Sip_Transaction::create(_dispatcher->get_sip_stack_internal(),
                                                      req,
                                                      cmd.get_source() == Sip_SMCommand::dialog_layer,
                                                      _handle_ack);
        }
    }
    if( new_transaction )
    {
        add_transaction(new_transaction);
        bool handled = new_transaction->handle_command(cmd);
        if( !handled )
            std::cerr <<"ERROR: TransactionLayer::defaultCommandHandler: Transaction refused command: "<< cmd <<std::endl;
        return handled;
    }
    else
    {
        std::cerr <<"ERROR: TransactionLayer::defaultCommandHandler: Could not handle: "<< cmd <<std::endl;
        return false;
    }
}

void Sip_Layer_Transaction::do_handle_ack(bool b)
{
    _handle_ack = b;
}

SRef<Sip_Transaction*> Sip_Layer_Transaction::get_transaction(std::string transaction_id)
{
    std::map<std::string, SRef<Sip_Transaction*> >::iterator ft = _transactions.find(transaction_id);
    if( ft == _transactions.end() )
    {
        SRef<Sip_Transaction*> null;
        return null;
    }
    else
        return (*ft).second;
}

void Sip_Layer_Transaction::add_transaction(SRef<Sip_Transaction*> t)
{
    my_assert(t->get_branch().size() > 0);
    _transactions[t->get_transaction_id()] = t;
}

void Sip_Layer_Transaction::remove_transaction(std::string transaction_id)
{
    _transactions[transaction_id]->free_state_machine();
    int rtn = (int)_transactions.erase(transaction_id);
    my_assert(rtn == 1); //?
}



std::list<SRef<Sip_Transaction*> > Sip_Layer_Transaction::get_transactions()
{
    std::list<SRef<Sip_Transaction*> > ret;
    std::map<std::string, SRef<Sip_Transaction*> >::iterator i;
    for(i = _transactions.begin(); i != _transactions.end(); ++i)
        ret.push_back((*i).second);
    return ret;
}

std::list<SRef<Sip_Transaction*> > Sip_Layer_Transaction::get_transactions_with_call_id(std::string callid)
{
    std::list<SRef<Sip_Transaction*> > ret;
    std::map<std::string, SRef<Sip_Transaction*> >::iterator i;
    for(i = _transactions.begin(); i != _transactions.end(); ++i)
    {
        if( (*i).second->get_call_id() == callid )
            ret.push_back((*i).second);
    }
    return ret;
}

bool Sip_Layer_Transaction::handle_command(const Sip_SMCommand &cmd)
{
    my_assert(cmd.get_destination() == Sip_SMCommand::transaction_layer);
#ifdef DEBUG_OUTPUT
    my_dbg("signaling/sip") << " Sip_Layer_Transaction::handleCommand got: "<< cmd <<std::endl;
#endif

    std::string tid;
    if( cmd.get_type() == Sip_SMCommand::COMMAND_STRING )
    {
        tid = cmd.get_command_string().get_destination_id();
    }

    if( cmd.get_type() == Sip_SMCommand::COMMAND_PACKET )
    {
        std::string branch = cmd.get_command_packet()->get_branch();
        if( branch.size() > 0 )
            tid = branch + cmd.get_command_packet()->get_cseq_method();
    }

    SRef<Sip_Transaction*> t;
    if( tid.size() > 0 )
    {
        t = get_transaction(tid);
        if( !t )
            t = get_transaction(tid + "c");
        if( !t )
            t = get_transaction(tid + "s");
    }

    if( t )
    {
        t->handle_command(cmd);
        return true;
    }
    else
    {
        std::string branch;
        std::string seq_method;

        if( cmd.get_type() == Sip_SMCommand::COMMAND_PACKET )
        {
            branch = cmd.get_command_packet()->get_branch();
            seq_method = cmd.get_command_packet()->get_cseq_method();
        }

        bool has_branch = (branch != "");
        bool has_seq_method = (seq_method != "");

        if( !has_branch)
            my_dbg("signaling/sip") <<  "WARNING: SipLayerTransaction::handleCommand "
                                      "could not find branch parameter from packet - trying all transactions"<<std::endl;

        std::map<std::string, SRef<Sip_Transaction*> >::iterator iter = _transactions.begin();
        std::map<std::string, SRef<Sip_Transaction*> >::iterator iter_end = _transactions.end();
        while( iter != iter_end )
        {
            if((!has_branch || (*iter).second->get_branch() == branch || seq_method == "ACK") &&
                    ( !has_seq_method || (*iter).second->get_cseq_method() == seq_method ||
                      (cmd.get_command_packet()->get_type() != Sip_Response::type && seq_method == "ACK" &&
                       (*iter).second->get_cseq_method() == "INVITE")))
            {
                bool ret = (*iter).second->handle_command(cmd);
                if(ret)
                    return true;
            }
            ++iter;
        }
    }
    return default_command_handler(cmd);
}
