#ifndef SIP_HEADER_VALUE_CONTENT_LENGTH_H
#define SIP_HEADER_VALUE_CONTENT_LENGTH_H

#include "sip_header.h"

extern Sip_Header_Factory_Func_Ptr sip_header_content_length_factory;

class Sip_Header_Value_Content_Length : public Sip_Header_Value
{
public:
    Sip_Header_Value_Content_Length(int32_t length);
    Sip_Header_Value_Content_Length(const std::string &build_from);

    virtual ~Sip_Header_Value_Content_Length();

    virtual std::string get_mem_object_type() const {return "SipHeaderContentLength";}
    virtual std::string get_string() const;
    int32_t get_content_length() const;

    void set_content_length(int32_t content_length);

private:
    int32_t _content_length;
};

#endif // SIP_HEADER_VALUE_CONTENT_LENGTH_H
