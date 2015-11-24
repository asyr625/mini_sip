#ifndef IP_PROVIDER_H
#define IP_PROVIDER_H

#include "my_types.h"
#include "sobject.h"

class UDP_Socket;
class Sip_Configuration;

class Ip_Provider : public SObject
{
    static std::string obtain_local_ip_address(SRef<Sip_Configuration *> config, bool& isIPv6);
public:
    virtual std::string get_external_ip() = 0;
    virtual uint16_t get_external_port( SRef<UDP_Socket *> sock ) = 0;
    virtual void set_external_ip(const std::string& _external_ip) = 0;

    static SRef<Ip_Provider *> create( SRef<Sip_Configuration *> config, bool useIpv6 = false );
};

#endif // IP_PROVIDER_H
