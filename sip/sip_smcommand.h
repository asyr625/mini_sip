#ifndef SIP_SMCOMMAND_H
#define SIP_SMCOMMAND_H
#include"sobject.h"
#include "command_string.h"
#include "sip_message.h"
#include "dbg.h"

class Sip_SMCommand : public SObject
{
public:
    static const int COMMAND_PACKET;
    static const int COMMAND_STRING;
    static const int transport_layer;
    static const int dialog_layer;
    static const int transaction_layer;
    static const int dispatcher;

    Sip_SMCommand(const Command_String& cmd, int source, int destination);
    Sip_SMCommand(SRef<Sip_Message*> p, int source, int destination);

    virtual std::string get_mem_object_type() const { return "SipSMCommand"; }

    int get_type() const;

    int get_source() const;
    void set_source(int s);

    int get_destination() const;

    void set_destination(int s);

    std::string get_destination_id() const;

    SRef<Sip_Message*> get_command_packet() const;

    Command_String get_command_string() const;

    friend Dbg& operator<< (Dbg&, const Sip_SMCommand&);

    friend std::ostream& operator<< (std::ostream&, const Sip_SMCommand&);

private:
    int _type;
    Command_String _cmd_str;
    SRef<Sip_Message*> _cmd_pkt;
    int _source;
    int _destination;
};


class Sip_SMCommand_Receiver : public virtual SObject
{
public:
    virtual bool handle_command(const Sip_SMCommand& cmd) = 0;
};

#endif // SIP_SMCOMMAND_H
