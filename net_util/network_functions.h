#ifndef NETWORK_FUNCTIONS_H
#define NETWORK_FUNCTIONS_H
#include <vector>
#include "sobject.h"

class Network_Interface : public SObject
{
public:
    const std::string &get_name() const;

    const std::vector<std::string> &get_ip_strings( bool ipv6 = false ) const;

    void add_ip_string( const std::string &ip, bool ipv6 = false );

    Network_Interface(const std::string &name);
    ~Network_Interface();

private:
    std::string m_name;
    std::vector<std::string> m_ip4Strs;
    std::vector<std::string> m_ip6Strs;
};


class Network_Functions
{
public:
    static void init();

    static std::vector<std::string> get_all_interfaces();

    static std::string get_interface_ipstr(std::string iface);

    static std::vector<SRef<Network_Interface*> > get_interfaces();

    static std::string get_interface_of(std::string ipStr);

    static std::string get_host_handling_service(std::string service, std::string domain, unsigned short &ret_port);

    static bool is_local_ip(unsigned int ip, std::vector<std::string> &localIPs);

    static void bin_ip2string(unsigned int ip, char *strBufMin16);
};

#endif // NETWORK_FUNCTIONS_H
