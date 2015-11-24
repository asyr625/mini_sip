#ifndef SIP_HEADER_FROM_H
#define SIP_HEADER_FROM_H

#include "sip_uri.h"
#include "sip_header.h"

extern Sip_Header_Factory_Func_Ptr sip_header_from_factory;

class Sip_Header_Value_From : public Sip_Header_Value
{
public:
    Sip_Header_Value_From(const std::string &build_from);
    Sip_Header_Value_From(const Sip_Uri& uri);

    virtual ~Sip_Header_Value_From();

    virtual std::string get_mem_object_type() const {return "SipHeaderFrom";}
    std::string get_string() const;
    Sip_Uri get_uri() const;
    void set_uri(const Sip_Uri &uri);

private:
    Sip_Uri _uri;
};

#endif // SIP_HEADER_FROM_H
