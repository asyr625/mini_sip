#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H

#include "stream_socket.h"

class Server_Socket : public Socket
{
public:
    virtual SRef<Stream_Socket *> accept();
    void listen(const IPAddress &addr, int port, int backlog );

protected:
    Server_Socket();
    Server_Socket( int domain, int type, int protocol );

    /**
     * Used in accept to create a socket for
     * the incoming connection.
     */
    virtual SRef<Stream_Socket *> create_socket( int sd, struct sockaddr *sa, int salen )=0;
};

#endif // SERVER_SOCKET_H
