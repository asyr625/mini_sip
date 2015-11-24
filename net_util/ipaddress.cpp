#include "ipaddress.h"
#include "ip4address.h"
#ifdef HAVE_IPV6
#include "ip6address.h"
#endif
#include "network_exception.h"

#include "net_config.h"

#include "my_defines.h"

#ifdef WIN32
# include<winsock2.h>
# include<ws2tcpip.h>
#elif defined HAVE_NETDB_H
# include<sys/types.h>
# include<sys/socket.h>
# include<arpa/inet.h>
#endif
#ifndef HAVE_INET_PTON
# include "inet_pton.h"
#endif

using namespace std;

IPAddress::~IPAddress()
{
}

int IPAddress::get_address_family() const
{
    return _address_family;
}

void IPAddress::set_address_family(int af)
{
    _address_family = af;
}

int IPAddress::get_protocol_family() const
{
    return _protocol_family;
}

void IPAddress::set_protocol_family(int pf)
{
    _protocol_family = pf;
}


SRef<IPAddress *> IPAddress::create( struct sockaddr * addr, int32_t addr_len )
{
    if( addr->sa_family == AF_INET &&
        addr_len >= (int32_t) sizeof(struct sockaddr_in))
    {
        return new IP4Address( (sockaddr_in *)addr );
    }
#ifdef HAVE_IPV6
    else if( addr->sa_family == AF_INET6 &&
         addr_len >= (int32_t)sizeof(struct sockaddr_in6))
    {
        return new IP6Address( (sockaddr_in6 *)addr );
    }
#endif
    else
        throw Unknown_Address_Family(addr->sa_family);
}

SRef<IPAddress *> IPAddress::create(const string &addr)
{
    try {
        return new IP4Address( addr );
    } catch( Host_Not_Found & )
    { }
#ifdef HAVE_IPV6
    try {
        return new IP6Address( addr );
    } catch( Host_Not_Found & ){ }
#endif
    return NULL;
}

SRef<IPAddress *> IPAddress::create(const std::string &addr, bool use_ipv6)
{
    try {
        if( !use_ipv6 )
            return new IP4Address( addr );
#ifdef HAVE_IPV6
        else
            return new IP6Address( addr );
#endif
    } catch( Host_Not_Found & ){ }
    return NULL;
}

bool IPAddress::is_numeric(const string &addr)
{
    char tmp[sizeof(struct in6_addr)];

    if( (inet_pton(AF_INET, addr.c_str(), &tmp) > 0) ||
        (inet_pton(AF_INET6, addr.c_str(), &tmp) > 0) )
    {
        return true;
    }
    return false;
}
