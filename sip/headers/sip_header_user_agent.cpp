#include "sip_header_user_agent.h"

SRef<Sip_Header_Value *> useragentFactory(const std::string &build_from)
{
    return new Sip_Header_Value_User_Agent(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_user_agent_factory = useragentFactory;

const std::string sipHeaderValueTypeUserAgentTypeStr = "User-Agent";

Sip_Header_Value_User_Agent::Sip_Header_Value_User_Agent(const std::string &build_from)
    : Sip_Header_Value_String(SIP_HEADER_TYPE_USERAGENT,sipHeaderValueTypeUserAgentTypeStr, build_from)
{
}
