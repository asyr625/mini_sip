#ifndef IP4_SERVER_SOCKET_H
#define IP4_SERVER_SOCKET_H

#include "tcp_socket.h"
#include "server_socket.h"


class IP4_Server_Socket : public Server_Socket
{
public:
    IP4_Server_Socket(int listenport, int backlog=25);

    virtual std::string get_mem_object_type() const {return "IP4ServerSocket";}

    //inherited: TCPSocket *accept();
    virtual struct sockaddr *get_sockaddr_struct(int &ret_length);

    virtual SRef<TCP_Socket *> create_socket(int fd, struct sockaddr_in *saddr);
};

#endif // IP4_SERVER_SOCKET_H
