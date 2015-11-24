#include "tcp_server_socket.h"
#include "tcp_socket.h"

#ifdef WIN32
#include<winsock2.h>
#elif defined HAVE_NETDB_H
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#endif

#ifndef _MSC_VER
#include<unistd.h>
#endif
using namespace std;

Tcp_Server_Socket::Tcp_Server_Socket(int domain)
    :Server_Socket( domain, SOCK_STREAM, IPPROTO_TCP )
{
    _type = SSOCKET_TYPE_TCP;
}

Tcp_Server_Socket *Tcp_Server_Socket::create( int listenport, bool useIpv6, int backlog )
{
    SRef<IPAddress*> addr;

    if( useIpv6 )
        addr = IPAddress::create("::", true);
    else
        addr = IPAddress::create("0.0.0.0", false);

    int32_t domain = addr->get_address_family();

    Tcp_Server_Socket* sock =
        new Tcp_Server_Socket( domain );
    sock->listen( **addr, listenport, backlog );
    return sock;
}

// ServerSocket
SRef<Stream_Socket *> Tcp_Server_Socket::create_socket( int fd, struct sockaddr *sa, int salen )
{
    return new TCP_Socket( fd, sa, salen );
}


bool Tcp_Server_Socket::set_ipv6_dscp(const std::bitset<6> &dscp)
{
    int trafficClass = dscp.to_ulong() << 2;
    if(setsockopt(_fd, IPPROTO_IPV6, IPV6_TCLASS, &trafficClass, sizeof(trafficClass)))
    {
        cerr << "(Tcp_Server_Socket::set_ipv6_dscp)UDPSocket::setDSCP() error, could not set DSCP."<< endl;
        return false;
    }
    return true;
}
