#include <iostream>

#include "sip_transport_tls.h"

#include "tls_socket.h"
#include "tls_server_socket.h"
#include "tcp_socket.h"
#include "tcp_server_socket.h"

#include "network_exception.h"

Sip_Transport_Tls::Sip_Transport_Tls( SRef<Library *> lib )
    : Sip_Transport(lib)
{
}

Sip_Transport_Tls::~Sip_Transport_Tls()
{
}


unsigned int Sip_Transport_Tls::get_version() const
{
    return 0x00000001;
}

SRef<Sip_Socket_Server *> Sip_Transport_Tls::create_server( SRef<Sip_Socket_Receiver*> receiver,
                                                  bool ipv6, const std::string &ip_string,
                                                  int &pref_port, SRef<Certificate_Set *> cert_db,
                                                  SRef<Certificate_Chain *> cert_chain)
{
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

    SRef<Server_Socket*> ssock = Tcp_Server_Socket::create( port, ipv6 );
    sock = Tls_Server_Socket::create( ssock, cert_chain->get_first(), cert_db );

    server = new Stream_Socket_Server( receiver, sock );
    server->set_external_ip( ip_string );
    pref_port = sock->get_port();

    return server;
}

SRef<Stream_Socket *> Sip_Transport_Tls::connect( const IPAddress &addr, uint16_t port,
                                                  SRef<Certificate_Set *> cert_db,
                                                  SRef<Certificate_Chain *> cert_chain)
{
    SRef<Stream_Socket*> sock = new TCP_Socket( addr, port );
    return Tls_Socket::connect( sock, cert_chain->get_first(), cert_db );
}
