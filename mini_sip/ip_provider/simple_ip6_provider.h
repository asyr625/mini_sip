#ifndef SIMPLE_IP6_PROVIDER_H
#define SIMPLE_IP6_PROVIDER_H

#include "ip_provider.h"

class Simple_Ip6_Provider : public Ip_Provider
{
public:
    Simple_Ip6_Provider( SRef<Sip_Configuration *> config );

    virtual std::string get_external_ip();
    virtual std::string get_local_ip();
    virtual void set_external_ip(const std::string& _externalIP);
    virtual uint16_t get_external_port( SRef<UDP_Socket *> sock );

    virtual std::string get_mem_object_type() const {return "SimpleIp6Provider";}
    enum Scope
    {
        INVALID = 0,
        LINK_LOCAL = 2,		// fe80::/10
        SITE_LOCAL = 5,		// fec0::/10
        UNIQUE_LOCAL = 7,	// fc00::/7
        GLOBAL = 0xE		// 2000::/3
    };

    static Scope ip_scope( std::string ip );

private:
    std::string _local_ip;
};

#endif // SIMPLE_IP6_PROVIDER_H
