#ifndef SIP_HEADER_WARNING_H
#define SIP_HEADER_WARNING_H

#include "sip_header.h"

extern Sip_Header_Factory_Func_Ptr sip_header_warning_factory;

class Sip_Header_Value_Warning : public Sip_Header_Value
{
public:
    Sip_Header_Value_Warning(std::string domainName, uint16_t errorCode, std::string warning);
    Sip_Header_Value_Warning(const std::string &build_from);

    virtual ~Sip_Header_Value_Warning();

    virtual std::string get_mem_object_type() const {return "SipHeaderWarning";}
    std::string get_string() const;
    std::string get_warning() const;

    void set_warning(const std::string &ua);

    uint16_t get_error_code() const;

    void set_error_code(const uint16_t &errorCodec);

    std::string get_domain_name() const;

    void set_domain_name(const std::string &domainName);

private:
    std::string _warning;
    uint16_t _error_code;
    std::string _domain_name;
};

#endif // SIP_HEADER_WARNING_H
