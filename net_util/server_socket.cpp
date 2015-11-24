#include "server_socket.h"
#include "network_exception.h"
#include "net_config.h"
#include "my_error.h"

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

Server_Socket::Server_Socket()
{
}

Server_Socket::Server_Socket( int domain, int type, int protocol )
{
    _fd = (int32_t)::socket( domain, type, protocol );
    if (_fd<0)
    {
        throw Socket_Failed( errno );
    }
    int32_t on=1;
#ifdef WIN32
    setsockopt(_fd,SOL_SOCKET,SO_REUSEADDR, (const char *) (&on),sizeof(on));
#else
    setsockopt(_fd,SOL_SOCKET,SO_REUSEADDR, (void *) (&on),sizeof(on));
#endif

#ifdef IPV6_V6ONLY
    if( domain == PF_INET6 )
        setsockopt(_fd, IPPROTO_IPV6, IPV6_V6ONLY, &on, sizeof(on));
#endif
}

SRef<Stream_Socket *> Server_Socket::accept()
{
    int32_t cli;
    struct sockaddr_storage sin;
    socklen_t sinlen = sizeof(sin);
    //sin = get_sockaddr_struct(sinlen);

    if ((cli=(int32_t)::accept(_fd, (struct sockaddr*)&sin, &sinlen))<0){
        my_error("in Server_Socket::accept(): accept:");
    }

    return create_socket( cli, (struct sockaddr*)&sin, sinlen );
}

void Server_Socket::listen( const IPAddress &addr, int port, int backlog )
{
    socklen_t salen = addr.get_sockaddr_length();
    struct sockaddr *sa = addr.get_sockaddr_ptr( port );

    if (bind(_fd, sa, salen )!=0)
    {
        throw Bind_Failed( errno );
    }

    if (::listen(_fd, backlog)!=0)
    {
        throw Listen_Failed( errno );
    }
}
