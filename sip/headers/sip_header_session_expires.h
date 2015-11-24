#ifndef SIP_HEADER_SESSION_EXPIRES_H
#define SIP_HEADER_SESSION_EXPIRES_H

#include "sip_header.h"

extern Sip_Header_Factory_Func_Ptr sip_header_session_expires_factory;

class Sip_Header_Value_Session_Expires : public Sip_Header_Value
{
public:
    Sip_Header_Value_Session_Expires(const std::string &build_from);

    virtual ~Sip_Header_Value_Session_Expires();

    virtual std::string get_mem_object_type() const { return "SipHeaderValueSessionExpires"; }
    std::string get_string() const;

    uint32_t get_timeout() const;
private:
    uint32_t timeout;
};

#endif // SIP_HEADER_SESSION_EXPIRES_H
