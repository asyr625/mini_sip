
#include "net_config.h"
#include "sctp_socket.h"
#include "network_exception.h"

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

Sctp_Socket::Sctp_Socket( const IPAddress &addr, int port )
{
    _type = SSOCKET_TYPE_SCTP;
    _peer_address = addr.clone();
    _peer_port = port;

    if ((_fd = (int32_t)::socket(addr.get_protocol_family(), SOCK_STREAM, IPPROTO_SCTP ))<0)
    {
        throw Socket_Failed( errno );
    }
    int32_t on=1;
#ifndef WIN32
    setsockopt(_fd,SOL_SOCKET,SO_REUSEADDR, (void *) (&on),sizeof(on));
#else
    setsockopt(_fd,SOL_SOCKET,SO_REUSEADDR, (const char *) (&on),sizeof(on));
#endif

    addr.connect(*this, port);
}

Sctp_Socket::Sctp_Socket(int fd_, struct sockaddr * addr, int addr_len)
{
    _type = SSOCKET_TYPE_SCTP;
    _fd = fd_;
    _peer_address = IPAddress::create( addr, addr_len );
    _peer_port = _peer_address->get_port();
}

Sctp_Socket::~Sctp_Socket()
{
}
