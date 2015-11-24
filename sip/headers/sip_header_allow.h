#ifndef SIP_HEADER_ALLOW_H
#define SIP_HEADER_ALLOW_H

#include "sip_header_string.h"

extern Sip_Header_Factory_Func_Ptr sip_header_allow_factory;

class Sip_Header_Value_Allow : public Sip_Header_Value_String
{
public:
    Sip_Header_Value_Allow(const std::string &build_from);

    virtual std::string get_mem_object_type() const { return "SipHeaderAllow"; }

};

#endif // SIP_HEADER_ALLOW_H
