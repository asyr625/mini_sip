#ifndef SCTP_SERVER_SOCKET_H
#define SCTP_SERVER_SOCKET_H

#include "server_socket.h"

class Sctp_Server_Socket : public Server_Socket
{
public:
    static Sctp_Server_Socket *create( int listenport,
                    bool useIpv6=false,
                    int backlog=25 );

    // ServerSocket
    virtual SRef<Stream_Socket *> create_socket( int fd,
                           struct sockaddr *sa,
                           int salen );

    // MObject
    virtual std::string get_mem_object_type() const {return "SctpServerSocket";}

protected:
    Sctp_Server_Socket( const IPAddress &addr );
};

#endif // SCTP_SERVER_SOCKET_H
