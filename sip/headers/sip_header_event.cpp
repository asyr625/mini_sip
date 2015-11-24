#include "sip_header_event.h"

SRef<Sip_Header_Value *> eventFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Event(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_event_factory = eventFactory;

const std::string sipHeaderValueEventTypeStr = "Event";

Sip_Header_Value_Event::Sip_Header_Value_Event(const std::string &build_from)
    : Sip_Header_Value_String(SIP_HEADER_TYPE_EVENT,sipHeaderValueEventTypeStr,build_from)
{
}
