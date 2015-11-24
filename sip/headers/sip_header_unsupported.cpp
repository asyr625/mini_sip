#include "sip_header_unsupported.h"


SRef<Sip_Header_Value *> unsupportedFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Unsupported(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_unsupported_factory = unsupportedFactory;

const std::string SipHeaderValueUnsupportedTypeStr = "Unsupported";

Sip_Header_Value_Unsupported::Sip_Header_Value_Unsupported(const std::string &build_from)
    : Sip_Header_Value_String(SIP_HEADER_TYPE_UNSUPPORTED, SipHeaderValueUnsupportedTypeStr, build_from)
{
}
