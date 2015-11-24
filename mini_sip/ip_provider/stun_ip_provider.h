#ifndef STUN_IP_PROVIDER_H
#define STUN_IP_PROVIDER_H

#include "ip_provider.h"
#include "ipaddress.h"

class Stun_Ip_Provider : public Ip_Provider
{
public:
    static SRef<Ip_Provider *> create( SRef<Sip_Configuration *> config );

    virtual std::string get_external_ip();
    virtual void set_external_ip(const std::string& external_ip);

    virtual uint16_t get_external_port(SRef<UDP_Socket *> socket );

    virtual std::string get_mem_object_type() const { return "StunIpProvider"; }

private:
    Stun_Ip_Provider( uint32_t natType, std::string externalIp, SRef<IPAddress *> stunIp, uint16_t stunPort );
    virtual ~Stun_Ip_Provider();

    SRef<IPAddress *> _stun_ip;
    uint16_t _stun_port;
    std::string _external_ip;
    uint32_t _nat_type;
};

#endif // STUN_IP_PROVIDER_H
