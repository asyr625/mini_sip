#include "sip_transport_sctp.h"

#include "sctp_socket.h"
#include "sctp_server_socket.h"
#include "network_exception.h"

Sip_Transport_Sctp::Sip_Transport_Sctp( SRef<Library *> lib )
    : Sip_Transport(lib)
{
}


Sip_Transport_Sctp::~Sip_Transport_Sctp()
{
}

unsigned int Sip_Transport_Sctp::get_version() const
{
    return 0x00000001;
}


SRef<Sip_Socket_Server *> Sip_Transport_Sctp::create_server( SRef<Sip_Socket_Receiver*> receiver,
                                                  bool ipv6, const std::string &ip_string,
                                                  int &pref_port, SRef<Certificate_Set *>,
                                                  SRef<Certificate_Chain *>)
{
    SRef<Server_Socket *> sock;
    SRef<Sip_Socket_Server *> server;
    int port = pref_port;

    try {
        sock = Sctp_Server_Socket::create( port, ipv6 );
    }
    catch ( const Bind_Failed &bf ){
        sock = Sctp_Server_Socket::create( 0, ipv6 );
    }

    server = new Stream_Socket_Server( receiver, sock );
    server->set_external_ip( ip_string );

    pref_port = sock->get_port();

    return server;
}


SRef<Stream_Socket *> Sip_Transport_Sctp::connect( const IPAddress &addr, uint16_t port,
                                          SRef<Certificate_Set *> ,
                                          SRef<Certificate_Chain *> )
{
    return new Sctp_Socket( addr, port );
}
