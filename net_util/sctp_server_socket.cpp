#include "sctp_server_socket.h"
#include "sctp_socket.h"

Sctp_Server_Socket::Sctp_Server_Socket( const IPAddress &addr )
    : Server_Socket( addr.get_address_family(), SOCK_STREAM, IPPROTO_SCTP )
{
    _type = SSOCKET_TYPE_SCTP;
}

Sctp_Server_Socket *Sctp_Server_Socket::create( int32_t listenport, bool useIpv6, int32_t backlog )
{
    SRef<IPAddress*> addr;

    if( useIpv6 )
        addr = IPAddress::create("::", true);
    else
        addr = IPAddress::create("0.0.0.0", false);

    Sctp_Server_Socket* sock = new Sctp_Server_Socket( **addr );
    sock->listen( **addr, listenport, backlog );
    return sock;
}

SRef<Stream_Socket *> Sctp_Server_Socket::create_socket( int32_t fd_, struct sockaddr *sa, int32_t salen )
{
    return new Sctp_Socket( fd_, sa, salen );
}
