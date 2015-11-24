#include "sip_header_www_authenticate.h"


SRef<Sip_Header_Value *> wwwAuthFactory(const std::string &build_from)
{
    return new Sip_Header_Value_WWW_Authenticate(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_www_authenticate_factory = wwwAuthFactory;


const std::string sipHeaderValueWWWAuthenticationTypeStr = "WWW-Authenticate";

Sip_Header_Value_WWW_Authenticate::Sip_Header_Value_WWW_Authenticate(const std::string &build_from)
    : Sip_Header_Value_Proxy_Authenticate(SIP_HEADER_TYPE_WWWAUTHENTICATE,sipHeaderValueWWWAuthenticationTypeStr, build_from)
{
}

Sip_Header_Value_WWW_Authenticate::~Sip_Header_Value_WWW_Authenticate()
{
}
