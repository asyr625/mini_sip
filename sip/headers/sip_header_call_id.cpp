#include "sip_header_call_id.h"

SRef<Sip_Header_Value *> callidFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Call_ID(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_call_id_factory = callidFactory;

const std::string sipHeaderValueCallIdTypeStr = "Call-ID";

Sip_Header_Value_Call_ID::Sip_Header_Value_Call_ID(const std::string& i)
    : Sip_Header_Value_String(SIP_HEADER_TYPE_CALLID, sipHeaderValueCallIdTypeStr, i)
{
}
