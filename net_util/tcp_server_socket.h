#ifndef TCP_SERVER_SOCKET_H
#define TCP_SERVER_SOCKET_H

#include "server_socket.h"

class Tcp_Server_Socket : public Server_Socket
{
public:

    static Tcp_Server_Socket *create( int listenport,
                    bool useIpv6=false,
                    int backlog=25 );

    // ServerSocket
    virtual SRef<Stream_Socket *> create_socket( int fd,
                           struct sockaddr *sa,
                           int salen );

    // MObject
    virtual std::string get_mem_object_type() const {return "TcpServerSocket";}

    virtual bool set_ipv6_dscp(const std::bitset<6> &dscp);

protected:
    Tcp_Server_Socket( int domain );
};

#endif // TCP_SERVER_SOCKET_H
