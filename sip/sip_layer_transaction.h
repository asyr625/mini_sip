#ifndef SIP_LAYER_TRANSACTION_H
#define SIP_LAYER_TRANSACTION_H

#include "sip_smcommand.h"

#include "sip_layer_transport.h"

class Sip_Command_Dispatcher;
class Sip_Transaction;

class Sip_Request;


class Sip_Layer_Transaction : public Sip_SMCommand_Receiver
{
public:
    Sip_Layer_Transaction(SRef<Sip_Command_Dispatcher*> dispatcher,
            SRef<Sip_Layer_Transport*> transp);

    ~Sip_Layer_Transaction();

    void do_handle_ack(bool b);

    void remove_transaction(std::string transaction_id);

    SRef<Sip_Transaction*> get_transaction(std::string transaction_id);

    std::list<SRef<Sip_Transaction*> > get_transactions();

    std::list<SRef<Sip_Transaction*> > get_transactions_with_call_id(std::string callid);

    virtual std::string get_mem_object_type() const {return "SipLayerTransaction";}

    virtual bool handle_command(const Sip_SMCommand &cmd);

    std::string create_client_transaction( SRef<Sip_Request*> req );

private:
    void add_transaction(SRef<Sip_Transaction*> t);

    bool default_command_handler(const Sip_SMCommand &cmd);

    bool _handle_ack;

    std::map<std::string, SRef<Sip_Transaction*> > _transactions;

    SRef<Sip_Command_Dispatcher*> _dispatcher;
    SRef<Sip_Layer_Transport*> _transport_layer;
};

#endif // SIP_LAYER_TRANSACTION_H
