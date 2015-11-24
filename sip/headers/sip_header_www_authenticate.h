#ifndef SIP_HEADER_WWW_AUTHENTICATE_H
#define SIP_HEADER_WWW_AUTHENTICATE_H

#include "sip_header_proxy_authenticate.h"

extern Sip_Header_Factory_Func_Ptr sip_header_www_authenticate_factory;

class Sip_Header_Value_WWW_Authenticate : public Sip_Header_Value_Proxy_Authenticate
{
public:
    Sip_Header_Value_WWW_Authenticate(const std::string &build_from);

    virtual ~Sip_Header_Value_WWW_Authenticate();

    virtual std::string get_mem_object_type() const {return "SipHeaderWWWAuthenticate";}
};

#endif // SIP_HEADER_WWW_AUTHENTICATE_H
