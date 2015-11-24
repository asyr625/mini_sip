#ifndef SCTP_SOCKET_H
#define SCTP_SOCKET_H

#include "stream_socket.h"

class Sctp_Socket : public Stream_Socket
{
public:
    Sctp_Socket( const IPAddress &addr, int port );
    Sctp_Socket(int fd_, struct sockaddr * addr, int addr_len);

    virtual ~Sctp_Socket();

    virtual std::string get_mem_object_type() const {return "Sctp_Socket";}
};

#endif // SCTP_SOCKET_H
