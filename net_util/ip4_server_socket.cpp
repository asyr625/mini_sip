#include "ip4_server_socket.h"
#include <string.h>
#include "my_defines.h"
#include "server_socket.h"

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

IP4_Server_Socket::IP4_Server_Socket(int32_t listenport, int32_t backlog)
    : Server_Socket(PF_INET,listenport, 0)
{
    struct sockaddr_in sin;
    //bzero((char*)&sin, sizeof(sin));
    memset(&sin, '\0', sizeof(sin));
    sin.sin_family = AF_INET;
//	sin.sin_addr=0;//any
    sin.sin_port = htons( (unsigned short)listenport );
    this->listen(*(const IPAddress *)&sin,sizeof(sin),backlog);
}

struct sockaddr *IP4_Server_Socket::get_sockaddr_struct(int32_t &ret_length)
{
    ret_length = sizeof(struct sockaddr_in);
    struct sockaddr_in *sin = (struct sockaddr_in*) malloc (sizeof(struct sockaddr_in));
    return (struct sockaddr *)sin;
}

SRef<TCP_Socket *> IP4_Server_Socket::create_socket(int32_t fd_, struct sockaddr_in *saddr)
{
    return new TCP_Socket(fd_, (struct sockaddr*)saddr, sizeof(struct sockaddr_in));
}
