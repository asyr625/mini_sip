#include "sip_transport_dtls_udp.h"

#include "udp_socket.h"

#include "dtls_socket.h"
#

#include "network_exception.h"

Sip_Transport_Dtls_Udp::Sip_Transport_Dtls_Udp( SRef<Library *> lib )
    : Sip_Transport(lib)
{
}


Sip_Transport_Dtls_Udp::~Sip_Transport_Dtls_Udp()
{
}

unsigned int Sip_Transport_Dtls_Udp::get_version() const
{
    return 0x00000001;
}

SRef<Sip_Socket_Server *> Sip_Transport_Dtls_Udp::create_server( SRef<Sip_Socket_Receiver*> receiver,
                                                  bool ipv6, const std::string &ip_string,
                                                  int &pref_port, SRef<Certificate_Set *> cert_db,
                                                  SRef<Certificate_Chain *> cert_chain )
{
    int port = pref_port;
    SRef<Sip_Socket_Server *> server;

    SRef<Datagram_Socket *> sock = new UDP_Socket( port, ipv6 );

    SRef<Datagram_Socket *> dsock =
        Dtls_Socket::create( sock, cert_chain->get_first(), cert_db );

    server = new Datagram_Socket_Server( receiver, dsock );
    server->set_external_ip( ip_string );

    pref_port = sock->get_port();
    return server;
}
