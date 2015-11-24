#ifndef SIP_HEADER_TO_H
#define SIP_HEADER_TO_H

#include "sip_header.h"
#include "sip_uri.h"

extern Sip_Header_Factory_Func_Ptr sip_header_to_factory;

class Sip_Header_Value_To : public Sip_Header_Value
{
public:
    Sip_Header_Value_To(const std::string &build_from);
    Sip_Header_Value_To(const Sip_Uri& uri);

    virtual ~Sip_Header_Value_To();

    virtual std::string get_mem_object_type() const {return "SipHeaderTo";}
    std::string get_string() const;
    Sip_Uri get_uri() const;
    void set_uri(const Sip_Uri &uri);

private:
    Sip_Uri uri;
};

#endif // SIP_HEADER_TO_H
