#include "udp_socket.h"

#include "net_config.h"
#include "my_defines.h"

#if defined _MSC_VER || __MINGW32__
#include<winsock2.h>
# define USE_WIN32_API
#elif defined HAVE_NETINET_IN_H
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
//#include<netinet/tcp.h>
#include<netinet/in.h>
//#include<netdb.h>
#endif

#include "ipaddress.h"
#include "network_exception.h"
#include "my_assert.h"

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

using namespace std;

UDP_Socket::UDP_Socket( int port, bool use_ipv6)
{
    init_udp_socket( use_ipv6, port );
}

UDP_Socket::~UDP_Socket()
{
}

bool UDP_Socket::init_udp_socket( bool use_ipv6, int port )
{
    _type = SSOCKET_TYPE_UDP;
    this->use_ipv6 = use_ipv6;

#ifdef _MSC_VER
    my_assert( sizeof(SOCKET) == 4 );
#endif

    if ((_fd = (int32_t)socket(use_ipv6? PF_INET6:PF_INET, SOCK_DGRAM, IPPROTO_UDP ))<0)
    {
        #ifdef _WIN32_WCE //wince STLPort has not errno defined ... wcecompat has, though ...
        throw Socket_Failed( -1 );
        #else
        throw Socket_Failed( errno );
        #endif
    }

// I believe the following code should be removed, or enabled using a
// parameter. I think we should fail if the port is busy by default.
//	int32_t on=1;
//#ifndef WIN32
//	setsockopt(fd,SOL_SOCKET,SO_REUSEADDR, (void *) (&on),sizeof(on));
//#else
//	setsockopt(fd,SOL_SOCKET,SO_REUSEADDR, (const char *) (&on),sizeof(on));
//#endif


#ifdef HAVE_IPV6
    if (use_ipv6){
        struct sockaddr_in6 addr;
        int32_t on=1;

#ifdef IPV6_V6ONLY
        setsockopt(_fd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));
#endif

        memset(&addr, 0, sizeof(addr));
        addr.sin6_family=PF_INET6;
        addr.sin6_port=htons( (unsigned short)port );
        addr.sin6_addr=in6addr_any;

        if (bind(_fd, (struct sockaddr *)&addr, sizeof(addr))!=0)
        {
            throw Bind_Failed( errno );
        }

    }else
#endif // defined(HAVE_IPV6) && defined(HAVE_STRUCT_SOCKADDR_IN6)
    {
        struct sockaddr_in addr;
        addr.sin_family = PF_INET;
        addr.sin_port = htons( (unsigned short)port );
        addr.sin_addr.s_addr=htonl(INADDR_ANY);

        if (bind(_fd, (struct sockaddr *)&addr, sizeof(addr)) != 0)
        {
            throw Bind_Failed( errno );
        }
    }
    return true;
}

int UDP_Socket::sendto(const IPAddress &to_addr, int port, const void *msg, int len)
{
    if (use_ipv6 && ( to_addr.get_type() != IP_ADDRESS_TYPE_V6))
    {
        cerr << "Error: trying to send to IPv4 address using IPv6 socket" << endl;
        throw Send_Failed( errno );
    }
    if (!use_ipv6 && (to_addr.get_type() != IP_ADDRESS_TYPE_V4))
    {
        cerr << "Error: trying to send to IPv6 address using IPv4 socket" << endl;
        throw Send_Failed( errno );
    }

    SRef<IPAddress*> to_addr_copy = to_addr.clone(); //Workaround a bug in getSockaddrptr that _sets_ the port in the address

    return ::sendto(_fd, (const char*)msg, len, 0, to_addr_copy->get_sockaddr_ptr(port), to_addr_copy->get_sockaddr_length());
}

int UDP_Socket::recvfrom(void *buf, const int &len, SRef<IPAddress *> &from, int &port)
{
    struct sockaddr_storage sa;
    socklen_t sa_len = sizeof(sa);
    int n;

    n= ::recvfrom(_fd, (char *)buf,len, 0, (struct sockaddr*)&sa, &sa_len);
    from = IPAddress::create((struct sockaddr*)&sa, sa_len);
    port = from->get_port();
//	std::cout << "UDP_Socket::recvFrom() " << n << " bytes at port " << port << " at " << mtime() << "ms" << std::endl;
    return n;
}

int UDP_Socket::recv(void *buf, const int &len)
{
#ifndef WIN32
    socklen_t dummy=0;
    return ::recvfrom(_fd,buf,len,0,NULL,&dummy);
#else
    return ::recv(_fd, (char *)buf,len,0);
#endif
}

bool UDP_Socket::set_low_delay()
{
#ifdef USE_WIN32_API
    cerr << "Warning: setting IPTOS_LOWDELAY is not implemented for the Microsoft Visual compiler!"<< endl;
    return false;
#else
    int tos = IPTOS_LOWDELAY;
    if (setsockopt(_fd, IPPROTO_IP, IP_TOS, &tos, sizeof(tos)))
    {
        cerr << "WARNING: Could not set type of service to be time critical"<< endl;
        return false;
    }
    return true;
#endif
}

bool UDP_Socket::set_ipv6_dscp(const std::bitset<6> &dscp)
{
    int trafficClass = dscp.to_ulong() << 2;
    if(setsockopt(_fd, IPPROTO_IPV6, IPV6_TCLASS, &trafficClass, sizeof(trafficClass)))
    {
        cerr << "UDPSocket::setDSCP() error, could not set DSCP."<< endl;
        return false;
    }
    return true;
}


