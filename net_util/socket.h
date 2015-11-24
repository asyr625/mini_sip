#ifndef SOCKET_H
#define SOCKET_H

#define SSOCKET_TYPE_STREAM      0x10
#define SSOCKET_TYPE_TCP         0x11
#define SSOCKET_TYPE_TLS         0x12
#define SSOCKET_TYPE_TLSSRP      0x13
#define SSOCKET_TYPE_SCTP        0x14
#define SSOCKET_TYPE_TLS_SCTP    0x15

#define SSOCKET_TYPE_UDP         0x20

#include <bitset>

#include "sobject.h"
#include "ipaddress.h"


#include <stdlib.h>

#ifndef WIN32
#include <unistd.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#endif

#ifdef HAVE_WS2TCPIP_H
# include <ws2tcpip.h>
#endif

#include <iostream>

#ifdef USE_WIN32_API
typedef int socklen_t;
#endif

class Socket : public SObject
{
public:
    Socket();
    virtual ~Socket();
    virtual int get_fd();
    int get_type();

    virtual int get_port();

    virtual int get_address_family();

    virtual SRef<IPAddress *> get_local_address() const;

    virtual bool set_ipv6_dscp(const std::bitset<6> &dscp);

    virtual void close( void );
protected:
    int _type;
    int _fd;
};

#endif // SOCKET_H
