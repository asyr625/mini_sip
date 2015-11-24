#ifndef SIP_HEADER_UNSUPPORTED_H
#define SIP_HEADER_UNSUPPORTED_H

#include "sip_header_string.h"

extern Sip_Header_Factory_Func_Ptr sip_header_unsupported_factory;

class Sip_Header_Value_Unsupported : public Sip_Header_Value_String
{
public:
    Sip_Header_Value_Unsupported(const std::string &build_from);

    virtual std::string get_mem_object_type() const { return "SipHeaderUnsupported"; }
};

#endif // SIP_HEADER_UNSUPPORTED_H
