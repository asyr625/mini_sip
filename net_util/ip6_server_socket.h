#ifndef IP6_SERVER_SOCKET_H
#define IP6_SERVER_SOCKET_H

#include "tcp_socket.h"
#include "server_socket.h"

class IP6_Server_Socket : public Server_Socket
{
public:
    IP6_Server_Socket(int listenport, int backlog=25);

    virtual std::string get_mem_object_type() const {return "IP6ServerSocket";}

    //inherited: TCPSocket *accept();
    virtual struct sockaddr *get_sockaddr_struct(int &ret_length);

    virtual SRef<TCP_Socket *> create_socket(int fd_, struct sockaddr_in *saddr);
};

#endif // IP6_SERVER_SOCKET_H
