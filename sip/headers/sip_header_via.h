#ifndef SIP_HEADER_VIA_H
#define SIP_HEADER_VIA_H

#include "sip_header.h"

extern Sip_Header_Factory_Func_Ptr sip_header_via_factory;

class Sip_Header_Value_Via : public Sip_Header_Value
{
public:
    Sip_Header_Value_Via();
    Sip_Header_Value_Via(const std::string &build_from);
    Sip_Header_Value_Via(const std::string &proto, const std::string &ip, int32_t port);

    virtual ~Sip_Header_Value_Via();

    virtual std::string get_mem_object_type() const {return "SipHeaderVia";}
    std::string get_string() const;

    std::string get_protocol() const;
    void set_protocol(const std::string &protocol);

    std::string get_ip() const;
    void set_ip(const std::string &ip);

    int32_t get_port() const;
    void set_port(int32_t port);

private:
    std::string _protocol;
    std::string _ip;
    int32_t _port;
};

#endif // SIP_HEADER_VIA_H
