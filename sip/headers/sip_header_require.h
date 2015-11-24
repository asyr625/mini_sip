#ifndef SIP_HEADER_REQUIRE_H
#define SIP_HEADER_REQUIRE_H

#include "sip_header_string.h"

extern Sip_Header_Factory_Func_Ptr sip_header_require_factory;

class Sip_Header_Value_Require : public Sip_Header_Value_String
{
public:
    Sip_Header_Value_Require(const std::string &build_from);

    virtual std::string get_mem_object_type() const { return "SipHeaderRequire"; }
};

#endif // SIP_HEADER_REQUIRE_H
