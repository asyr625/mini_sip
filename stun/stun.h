#ifndef STUN_H
#define STUN_H

#include <vector>

#include "ipaddress.h"
#include "udp_socket.h"
#include "my_types.h"

class Stun
{
public:
    static const int STUN_ERROR;
    static const int STUNTYPE_BLOCKED;
    static const int STUNTYPE_OPEN_INTERNET;
    static const int STUNTYPE_FULL_CONE;
    static const int STUNTYPE_SYMMETRIC_NAT;
    static const int STUNTYPE_PORT_RESTRICTED;
    static const int STUNTYPE_RESTRICTED;
    static const int STUNTYPE_SYMMETRIC_FIREWALL;

    static int get_nat_type(IPAddress &stunAddr,
            uint16_t stunPort,
            UDP_Socket &socket,
//				IPAddress &localAddr,
                            std::vector<std::string> localIPs,
            uint16_t localPort,
            char *bufferMappedIP,
            uint16_t &mappedPort);

    static int get_nat_type(IPAddress &stunAddr,
            uint16_t stunPort,
            UDP_Socket &socket,
//				IPAddress &localAddr,
                            std::vector<std::string> localIPs,
            uint16_t localPort);

    static void get_external_mapping(IPAddress &stunAddr,
            uint16_t stunPort,
            UDP_Socket &socket,
            char *bufferMappedIP,
            uint16_t &mappedPort);

    static const char *type_to_string(int t);
};

#endif // STUN_H
