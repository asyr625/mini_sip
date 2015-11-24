#ifndef SIP_HEADER_CONTENT_TYPE_H
#define SIP_HEADER_CONTENT_TYPE_H

#include "sip_header_string.h"

extern Sip_Header_Factory_Func_Ptr sip_header_content_type_factory;

class Sip_Header_Value_Content_Type : public Sip_Header_Value_String
{
public:
    Sip_Header_Value_Content_Type(const std::string &build_from);

    virtual std::string get_mem_object_type() const { return "SipHeaderContentType"; }
};

#endif // SIP_HEADER_CONTENT_TYPE_H
