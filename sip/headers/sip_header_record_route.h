#ifndef SIP_HEADER_RECORD_ROUTE_H
#define SIP_HEADER_RECORD_ROUTE_H

#include "sip_header_string.h"

extern Sip_Header_Factory_Func_Ptr sip_header_record_route_factory;

class Sip_Header_Value_Record_Route : public Sip_Header_Value_String
{
public:
    Sip_Header_Value_Record_Route(const std::string &build_from);

    virtual std::string get_mem_object_type() const { return "SipHeaderRecordRoute"; }
};

#endif // SIP_HEADER_RECORD_ROUTE_H
