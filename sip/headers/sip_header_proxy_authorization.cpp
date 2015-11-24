#include "sip_header_proxy_authorization.h"

SRef<Sip_Header_Value *> proxyauthorizationFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Proxy_Authorization(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_proxy_authorization_factory = proxyauthorizationFactory;


const std::string sipHeaderValueProxyAuthorization = "Proxy-Authorization";

Sip_Header_Value_Proxy_Authorization::Sip_Header_Value_Proxy_Authorization(const std::string &build_from)
    : Sip_Header_Value_Authorization(SIP_HEADER_TYPE_PROXYAUTHORIZATION, build_from,sipHeaderValueProxyAuthorization)
{

}

Sip_Header_Value_Proxy_Authorization::Sip_Header_Value_Proxy_Authorization(
        const std::string &username,
        const std::string &realm,
        const std::string &nonce,
        const std::string &opaque,
        const Sip_Uri &uri,
        const std::string &response,
        const std::string &auth_method)
    : Sip_Header_Value_Authorization(SIP_HEADER_TYPE_PROXYAUTHORIZATION,
                                 username,
                                 realm,
                                 nonce,
                                 opaque,
                                 uri,
                                 response,
                                 auth_method,sipHeaderValueProxyAuthorization)
{
}

Sip_Header_Value_Proxy_Authorization::~Sip_Header_Value_Proxy_Authorization()
{
}


std::string Sip_Header_Value_Proxy_Authorization::get_string() const
{
    return Sip_Header_Value_Authorization::get_string();
}
