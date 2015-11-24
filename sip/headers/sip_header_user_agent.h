#ifndef SIP_HEADER_USER_AGENT_H
#define SIP_HEADER_USER_AGENT_H

#include "sip_header_string.h"

#define HEADER_USER_AGENT_DEFAULT	"Minisip"

extern Sip_Header_Factory_Func_Ptr sip_header_user_agent_factory;

class Sip_Header_Value_User_Agent : public Sip_Header_Value_String
{
public:
    Sip_Header_Value_User_Agent(const std::string &build_from);

    virtual std::string get_mem_object_type() const { return "SipHeaderUserAgent"; }
};

#endif // SIP_HEADER_USER_AGENT_H
