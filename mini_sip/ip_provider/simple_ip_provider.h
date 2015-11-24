#ifndef SIMPLE_IP_PROVIDER_H
#define SIMPLE_IP_PROVIDER_H

#include "ip_provider.h"

class Simple_Ip_Provider : public Ip_Provider
{
public:
    Simple_Ip_Provider( SRef<Sip_Configuration *> config );

    virtual std::string get_external_ip();

    virtual std::string get_local_ip();
    virtual uint16_t get_external_port( SRef<UDP_Socket *> sock );
    virtual void set_external_ip(const std::string& _external_ip);

    virtual std::string get_mem_object_type() const { return "SimpleIpProvider"; }
private:
    bool is_in_private_ip_range(std::string ipstr );
    std::string _local_ip;
};

#endif // SIMPLE_IP_PROVIDER_H
