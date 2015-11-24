#include <iostream>

#include "sip_smcommand.h"

const int Sip_SMCommand::COMMAND_PACKET=1;
const int Sip_SMCommand::COMMAND_STRING=2;
//const int Sip_SMCommand::remote=1;
const int Sip_SMCommand::dialog_layer=2;
const int Sip_SMCommand::transaction_layer=3;
//const int Sip_SMCommand::callback=4;
const int Sip_SMCommand::dispatcher=5;
const int Sip_SMCommand::transport_layer=6;

Sip_SMCommand::Sip_SMCommand(const Command_String& cmd, int source, int destination)
    : _type(COMMAND_STRING),
      _cmd_str(cmd),
      _cmd_pkt(NULL),
      _source(source),
      _destination(destination)
{
}

Sip_SMCommand::Sip_SMCommand(SRef<Sip_Message*> p, int source, int destination)
    : _type(COMMAND_PACKET),
      _cmd_str("",""),
      _cmd_pkt(p),
      _source(source),
      _destination(destination)
{
}

int Sip_SMCommand::get_type() const
{
    return _type;
}

int Sip_SMCommand::get_source() const
{
    return _source;
}

void Sip_SMCommand::set_source(int s)
{
    _source = s;
}

int Sip_SMCommand::get_destination() const
{
    return _destination;
}

void Sip_SMCommand::set_destination(int s)
{
    _destination = s;
}

std::string Sip_SMCommand::get_destination_id() const
{
    if( _type == Sip_SMCommand::COMMAND_PACKET )
        return _cmd_pkt->get_call_id();
    else
        return _cmd_str.get_destination_id();
}

SRef<Sip_Message*> Sip_SMCommand::get_command_packet() const
{
    return _cmd_pkt;
}

Command_String Sip_SMCommand::get_command_string() const
{
    return _cmd_str;
}


Dbg & operator<<(Dbg &o, const Sip_SMCommand &c)
{
    const char *s[6]={"(illegal)","dialog_layer","transaction_layer","(illegal)","dispatcher","transport_layer"};
    if (c._type==Sip_SMCommand::COMMAND_PACKET)
    {
        o <<"COMMAND_PACKET:"
         << (**c.get_command_packet()).get_description()
         <<" source="<< s[c._source-1]
        <<" dest="<<s[c._destination-1];
    }else{
        o <<"COMMAND_STRING:"<<c.get_command_string().get_string()
         <<",source="<< s[c._source-1]
        <<" dest="<<s[c._destination-1];

    }
    return o;
}

#ifndef _WIN32_WCE
std::ostream & operator<<(std::ostream &o, const Sip_SMCommand &c)
{
    const char *s[6]={"(illegal)","dialog_layer","transaction_layer","(illegal)","dispatcher","transport_layer"};
    if (c._type == Sip_SMCommand::COMMAND_PACKET)
    {
        o << "COMMAND_PACKET:"
         << (**c.get_command_packet()).get_description()
         <<" source="<< s[c._source-1]
        <<" dest="<<s[c._destination-1];
    }else{
        o <<"COMMAND_STRING:"<<c.get_command_string().get_string()
         <<",source="<< s[c._source-1]
        <<" dest="<<s[c._destination-1];

    }
    return o;
}
#endif
