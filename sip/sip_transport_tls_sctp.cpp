#include <iostream>

#include "sip_transport_tls_sctp.h"

#include "tls_socket.h"
#include "tls_server_socket.h"
#include "sctp_socket.h"
#include "sctp_server_socket.h"

#include "network_exception.h"

Sip_Transport_Tls_Sctp::Sip_Transport_Tls_Sctp( SRef<Library *> lib )
    : Sip_Transport(lib)
{
}

Sip_Transport_Tls_Sctp::~Sip_Transport_Tls_Sctp()
{
}


unsigned int Sip_Transport_Tls_Sctp::get_version() const
{
    return 0x00000001;
}

SRef<Sip_Socket_Server *> Sip_Transport_Tls_Sctp::create_server( SRef<Sip_Socket_Receiver*> receiver,
                                                  bool ipv6, const std::string &ip_string,
                                                  int &pref_port, SRef<Certificate_Set *> cert_db,
                                                  SRef<Certificate_Chain *> cert_chain)
{
    SRef<Server_Socket *> ssock;
    SRef<Server_Socket *> sock;
    SRef<Sip_Socket_Server *> server;
    int32_t port = pref_port;

    if( cert_chain.is_null() || cert_chain->get_first().is_null() )
    {
        my_err << "You need a personal certificate to run "
            "a TLS server. Please specify one in "
            "the certificate settings. minisip will "
            "now disable the TLS server." << std::endl;
        return NULL;
    }

    try {
        ssock = Sctp_Server_Socket::create( port, ipv6 );
    }
    catch ( const Bind_Failed &bf ){
        ssock = Sctp_Server_Socket::create( 0, ipv6 );
    }

    sock = Tls_Server_Socket::create( ssock, cert_chain->get_first(),
                    cert_db );
    server = new Stream_Socket_Server( receiver, sock );
    server->set_external_ip( ip_string );
    pref_port = ssock->get_port();

    return server;
}

SRef<Stream_Socket *> Sip_Transport_Tls_Sctp::connect( const IPAddress &addr, uint16_t port,
                                                  SRef<Certificate_Set *> cert_db,
                                                  SRef<Certificate_Chain *> cert_chain)
{
    SRef<Stream_Socket*> sock = new Sctp_Socket( addr, port );
    return Tls_Socket::connect( sock, cert_chain->get_first(), cert_db );
}
