#include "sip_header_require.h"

SRef<Sip_Header_Value *> requireFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Require(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_require_factory = requireFactory;

const std::string SipHeaderValueRequireTypeStr = "Require";

Sip_Header_Value_Require::Sip_Header_Value_Require(const std::string &build_from)
    : Sip_Header_Value_String(SIP_HEADER_TYPE_REQUIRE, SipHeaderValueRequireTypeStr, build_from)
{
}
