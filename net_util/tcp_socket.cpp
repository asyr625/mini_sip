#include "tcp_socket.h"
#include "net_config.h"
#include "my_defines.h"
#ifdef WIN32
#	include<winsock2.h>
#elif defined HAVE_NETINET_TCP_H
#	include<sys/types.h>
#	include<sys/socket.h>
#	include<netinet/tcp.h>
#	include<netinet/in.h>
#endif

#include<stdlib.h>
#include<stdio.h>
#ifdef _MSC_VER
#	include<io.h>
#	define dup		::_dup
#endif

#include "ipaddress.h"
#include "ip4address.h"
#include "network_exception.h"

#ifndef _MSC_VER
#include<unistd.h>
#endif

#include<iostream>

using namespace std;

TCP_Socket::TCP_Socket(std::string addr,int port)
{
    _remote_host_unresolved = addr;
    SRef<IPAddress *> tmp = IPAddress::create(addr);
    init_tcp_socket( **tmp, port );
}

TCP_Socket::TCP_Socket(const IPAddress &ipaddress,int port)
{
    init_tcp_socket( ipaddress, port );
}

void TCP_Socket::init_tcp_socket(const IPAddress &ipaddress, int port)
{
    _peer_address = ipaddress.clone();
    _peer_port = port;

    _type = SSOCKET_TYPE_TCP;
    if ((_fd = (int32_t)::socket(ipaddress.get_protocol_family(), SOCK_STREAM, IPPROTO_TCP ))<0)
    {
        throw Socket_Failed( errno );
    }
    int32_t on=1;
#ifndef WIN32
    setsockopt(_fd,SOL_SOCKET,SO_REUSEADDR, (void *) (&on),sizeof(on));
#else
    setsockopt(_fd,SOL_SOCKET,SO_REUSEADDR, (const char *) (&on),sizeof(on));
#endif

    ipaddress.connect(*this, port);
}


TCP_Socket::TCP_Socket(int fd, sockaddr * addr, int addr_len)
{
    _type = SSOCKET_TYPE_TCP;
    this->_fd=fd;
    _peer_address = IPAddress::create( addr, addr_len );
    _peer_port = _peer_address->get_port();
}

TCP_Socket::TCP_Socket(TCP_Socket &sock)
{
    _type = SSOCKET_TYPE_TCP;

#ifdef _WIN32_WCE
#	pragma message("LIBMUTIL::TCPSocket - COMPILATION Warning: dup(int fd) replacement not found for POCKET PC in EVC")
    this->_fd = sock._fd;
#else
    this->_fd = dup(sock._fd);
#endif

#ifdef DEBUG_OUTPUT
    cerr << "DEBUG: In TCPSocket copy constructor: First free descriptor number is " << this->_fd << endl;
#endif
}

TCP_Socket::~TCP_Socket()
{
    if (_fd != -1)
    {
        this->close();
        this->_fd = -1; //for debugging purpose (make sure error if using deleted pointer)
    }
}
void TCP_Socket::use_nodelay(bool noDelay)
{
    int on = noDelay?1:0;
#ifndef WIN32
    setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, (void*)(&on), sizeof(on));
#else
    setsockopt(_fd, IPPROTO_TCP, TCP_NODELAY, (const char *)(&on), sizeof(on));
#endif
}

bool TCP_Socket::set_ipv6_dscp(const std::bitset<6> &dscp)
{
    int trafficClass = dscp.to_ulong() << 2;
    if(setsockopt(_fd, IPPROTO_IPV6, IPV6_TCLASS, &trafficClass, sizeof(trafficClass)))
    {
        cerr << "UDPSocket::setDSCP() error, could not set DSCP."<< endl;
        return false;
    }
    return true;
}


