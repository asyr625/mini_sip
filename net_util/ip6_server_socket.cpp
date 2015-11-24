#include "ip6_server_socket.h"
#include "server_socket.h"

#include <string.h>
#ifdef WIN32
#include<winsock2.h>
#include<ws2tcpip.h>
#elif defined HAVE_NETDB_H
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<netdb.h>
#endif

#include<stdio.h>

#ifndef _MSC_VER
#include<unistd.h>
#endif

IP6_Server_Socket::IP6_Server_Socket(int listenport, int backlog)
    : Server_Socket(PF_INET6,listenport, 0)
{
    struct sockaddr_in6 sin;
    //bzero((char*)&sin, sizeof(sin));
    memset(&sin, '0', sizeof(sin));
    sin.sin6_family = AF_INET6;
    sin.sin6_addr = in6addr_any;
    sin.sin6_port = htons( (unsigned short)listenport );
    this->listen(*(const IPAddress  *)&sin,sizeof(sin),backlog);
}

struct sockaddr *IP6_Server_Socket::get_sockaddr_struct(int &ret_length)
{
    ret_length = sizeof(struct sockaddr_in6);
    struct sockaddr_in6 *sin = (struct sockaddr_in6*)malloc(sizeof(struct sockaddr_in6));
    return (struct sockaddr *)sin;
}

SRef<TCP_Socket *> IP6_Server_Socket::create_socket(int fd_, struct sockaddr_in *saddr)
{
    return new TCP_Socket(fd_,(struct sockaddr*)saddr, sizeof(struct sockaddr_in6));
}
