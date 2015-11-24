#ifndef SIP_HEADER_ACCEPT_H
#define SIP_HEADER_ACCEPT_H

#include "sip_header_string.h"

extern Sip_Header_Factory_Func_Ptr sip_header_accept_factory;

class Sip_Header_Value_Accept : public Sip_Header_Value_String
{
public:
    Sip_Header_Value_Accept(const std::string &build_from);

    virtual std::string get_mem_object_type() const { return "SipHeaderAccept"; }
};

#endif // SIP_HEADER_ACCEPT_H
