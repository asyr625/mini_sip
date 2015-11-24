#ifndef SIP_HEADER_AUTHENTICATION_INFO_H
#define SIP_HEADER_AUTHENTICATION_INFO_H

#include "sip_header_proxy_authenticate.h"

extern Sip_Header_Factory_Func_Ptr sip_header_authentication_info_factory;

class Sip_Header_Value_Authentication_Info : public Sip_Header_Value_Proxy_Authenticate
{
public:
    Sip_Header_Value_Authentication_Info(const std::string &build_from);

    virtual ~Sip_Header_Value_Authentication_Info();

    virtual std::string get_mem_object_type() const { return "SipHeaderAuthenticationInfo"; }
};

#endif // SIP_HEADER_AUTHENTICATION_INFO_H
