#ifndef SIP_HEADER_UNKNOWN_H
#define SIP_HEADER_UNKNOWN_H

#include "sip_header_string.h"


class Sip_Header_Value_Unknown : public Sip_Header_Value_String
{
public:
    Sip_Header_Value_Unknown(const std::string &headerName, const std::string &build_from);

    virtual std::string get_mem_object_type() const {return "SipHeaderUnknown";}
};

#endif // SIP_HEADER_UNKNOWN_H
