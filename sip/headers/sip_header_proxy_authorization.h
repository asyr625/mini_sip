#ifndef SIP_HEADER_PROXY_AUTHORIZATION_H
#define SIP_HEADER_PROXY_AUTHORIZATION_H

#include "sip_header_authorization.h"

extern Sip_Header_Factory_Func_Ptr sip_header_proxy_authorization_factory;

class Sip_Header_Value_Proxy_Authorization : public Sip_Header_Value_Authorization
{
public:
    Sip_Header_Value_Proxy_Authorization(const std::string &build_from);

    Sip_Header_Value_Proxy_Authorization(
            const std::string &username,
            const std::string &realm,
            const std::string &nonce,
            const std::string &opaque,
            const Sip_Uri &uri,
            const std::string &response,
            const std::string &auth_method="DIGEST");

    virtual ~Sip_Header_Value_Proxy_Authorization();

    virtual std::string get_mem_object_type() const {return "SipHeaderProxyAuthorization";}
    virtual std::string get_string() const;

protected:
    char get_first_parameter_separator() const {return ' ';}
    char get_parameter_separator() const {return ',';}
};

#endif // SIP_HEADER_PROXY_AUTHORIZATION_H
