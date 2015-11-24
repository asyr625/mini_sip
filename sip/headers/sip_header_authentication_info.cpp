#include "sip_header_authentication_info.h"


SRef<Sip_Header_Value *> authInfoFactory(const std::string &build_from)
{
  return new Sip_Header_Value_Authentication_Info(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_authentication_info_factory = authInfoFactory;

const std::string sipHeaderValueAuthenticationInfoTypeStr = "Authentication-Info";

Sip_Header_Value_Authentication_Info::Sip_Header_Value_Authentication_Info(const std::string &build_from)
        : Sip_Header_Value_Proxy_Authenticate(SIP_HEADER_TYPE_AUTHENTICATIONINFO, sipHeaderValueAuthenticationInfoTypeStr, build_from, false)
{
}

Sip_Header_Value_Authentication_Info::~Sip_Header_Value_Authentication_Info()
{
}
