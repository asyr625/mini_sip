#ifndef SIP_HEADER_EXPIRES_H
#define SIP_HEADER_EXPIRES_H

#include "sip_header.h"

extern Sip_Header_Factory_Func_Ptr sip_header_expires_factory;

class Sip_Header_Value_Expires : public Sip_Header_Value
{
public:
    Sip_Header_Value_Expires(int n=300);
    Sip_Header_Value_Expires(const std::string &build_from);

    virtual ~Sip_Header_Value_Expires();

    virtual std::string get_mem_object_type() const {return "SipHeaderExpires";}
    std::string get_string() const;
    int32_t get_timeout() const;
private:
    int32_t _timeout;
};

#endif // SIP_HEADER_EXPIRES_H
