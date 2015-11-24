#ifndef SIP_HEADER_SUPPORTED_H
#define SIP_HEADER_SUPPORTED_H

#include "sip_header_string.h"

extern Sip_Header_Factory_Func_Ptr sip_header_supported_factory;

class Sip_Header_Value_Supported : public Sip_Header_Value_String
{
public:
    Sip_Header_Value_Supported(const std::string &build_from);

    virtual std::string get_mem_object_type() const { return "SipHeaderSupported"; }
};

#endif // SIP_HEADER_SUPPORTED_H
