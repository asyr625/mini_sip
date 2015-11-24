#ifndef SIP_HEADER_VALUE_REFER_TO_H
#define SIP_HEADER_VALUE_REFER_TO_H

#include "sip_header_string.h"

extern Sip_Header_Factory_Func_Ptr sip_header_refer_to_factory;

class Sip_Header_Value_Refer_To : public Sip_Header_Value_String
{
public:
    Sip_Header_Value_Refer_To(const std::string &build_from);

    virtual std::string get_mem_object_type() const { return "SipHeaderReferTo"; }
};

#endif // SIP_HEADER_VALUE_REFER_TO_H
