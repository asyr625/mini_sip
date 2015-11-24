#include "sip_header_supported.h"

SRef<Sip_Header_Value *> supportedFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Supported(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_supported_factory = supportedFactory;

const std::string SipHeaderValueSupportedTypeStr = "Supported";

Sip_Header_Value_Supported::Sip_Header_Value_Supported(const std::string &build_from)
    : Sip_Header_Value_String(SIP_HEADER_TYPE_SUPPORTED,SipHeaderValueSupportedTypeStr, build_from)
{
}
