#ifndef SIP_HEADER_EVENT_H
#define SIP_HEADER_EVENT_H

#include "sip_header_string.h"

extern Sip_Header_Factory_Func_Ptr sip_header_event_factory;

class Sip_Header_Value_Event : public Sip_Header_Value_String
{
public:
    Sip_Header_Value_Event(const std::string &build_from);

    virtual std::string get_mem_object_type() const { return "SipHeaderEvent"; }
};

#endif // SIP_HEADER_EVENT_H
