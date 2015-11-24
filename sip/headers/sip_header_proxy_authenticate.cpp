#include "sip_header_proxy_authenticate.h"


SRef<Sip_Header_Value *> proxyauthFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Proxy_Authenticate(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_proxy_authenticate_factory = proxyauthFactory;


const std::string sipHeaderValueProxyAuthenticationTypeStr = "Proxy-Authenticate";

Sip_Header_Value_Proxy_Authenticate::Sip_Header_Value_Proxy_Authenticate(int type, const std::string &typeStr,
                                                                         const std::string &build_from, const bool &authMethodPresent)
    : Sip_Header_Value(type, typeStr), _auth_method("")
{
    init(build_from, authMethodPresent);
}

Sip_Header_Value_Proxy_Authenticate::Sip_Header_Value_Proxy_Authenticate(const std::string &build_from, const bool &authMethodPresent)
    : Sip_Header_Value(SIP_HEADER_TYPE_PROXYAUTHENTICATE,sipHeaderValueProxyAuthenticationTypeStr)
{
    init(build_from, authMethodPresent);
}

Sip_Header_Value_Proxy_Authenticate::~Sip_Header_Value_Proxy_Authenticate()
{
}


std::string Sip_Header_Value_Proxy_Authenticate::get_string() const
{
    return get_auth_method();
}

std::string Sip_Header_Value_Proxy_Authenticate::get_auth_method() const
{
    return _auth_method;
}


void Sip_Header_Value_Proxy_Authenticate::init(const std::string& build_from, const bool &authMethodPresent)
{
    if( authMethodPresent )
    {
        size_t pos = build_from.find_first_not_of(" \t\r\n");
        size_t last = build_from.find_first_of(" \t\r\n", pos);

        if( last != std::string::npos )
        {
            _auth_method = build_from.substr( pos, last - pos );
            add_parameter( new Sip_Header_Parameter( build_from.substr( last )));
        }
    }
    else
    {
        if(!build_from.empty())
            add_parameter(new Sip_Header_Parameter(build_from));
    }
}
