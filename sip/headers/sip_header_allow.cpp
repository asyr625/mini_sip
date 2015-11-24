#include "sip_header_allow.h"

SRef<Sip_Header_Value *> AllowFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Allow(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_allow_factory = AllowFactory;


const std::string sipHeaderValueAllowTypeStr = "Allow";

Sip_Header_Value_Allow::Sip_Header_Value_Allow(const std::string &build_from)
    : Sip_Header_Value_String(SIP_HEADER_TYPE_ALLOW,sipHeaderValueAllowTypeStr,build_from)
{
}
