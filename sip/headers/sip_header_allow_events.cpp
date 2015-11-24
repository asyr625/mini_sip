#include "sip_header_allow_events.h"

SRef<Sip_Header_Value *> allowEventsFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Allow_Events(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_allow_events_factory = allowEventsFactory;

const std::string sipHeaderValueAllowEventsTypeStr = "Allow-Events";

Sip_Header_Value_Allow_Events::Sip_Header_Value_Allow_Events(const std::string &build_from)
    : Sip_Header_Value_String(SIP_HEADER_TYPE_ALLOWEVENTS,sipHeaderValueAllowEventsTypeStr,build_from)
{
}
