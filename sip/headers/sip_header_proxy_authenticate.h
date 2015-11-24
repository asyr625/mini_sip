#ifndef SIP_HEADER_PROXY_AUTHENTICATE_H
#define SIP_HEADER_PROXY_AUTHENTICATE_H

#include "sip_header.h"

extern Sip_Header_Factory_Func_Ptr sip_header_proxy_authenticate_factory;

class Sip_Header_Value_Proxy_Authenticate : public Sip_Header_Value
{
public:
    Sip_Header_Value_Proxy_Authenticate(const std::string &build_from, const bool &authMethodPresent = true);

    virtual ~Sip_Header_Value_Proxy_Authenticate();

    virtual std::string get_mem_object_type() const {return "SipHeaderProxyAuthenticate";}
    std::string get_string() const;

    std::string get_auth_method() const;

protected:
    Sip_Header_Value_Proxy_Authenticate(int type, const std::string &typeStr, const std::string &build_from, const bool &authMethodPresent = true);

    char get_first_parameter_separator() const {return ' ';}
    char get_parameter_separator() const {return ',';}

private:
    void init(const std::string& build_from, const bool &authMethodPresent);

    std::string _auth_method;
};

#endif // SIP_HEADER_PROXY_AUTHENTICATE_H
