#include "sip_transport_udp.h"
#include "network_exception.h"
#include "udp_socket.h"


Sip_Transport_Udp::Sip_Transport_Udp( SRef<Library *> lib )
    : Sip_Transport(lib)
{
}

Sip_Transport_Udp::~Sip_Transport_Udp()
{
}

unsigned int Sip_Transport_Udp::get_version() const
{
    return 0x00000001;
}

SRef<Sip_Socket_Server *> Sip_Transport_Udp::create_server(SRef<Sip_Socket_Receiver*> receiver,
                                                  bool ipv6, const std::string &ip_string,
                                                  int &pref_port, SRef<Certificate_Set *>,
                                                  SRef<Certificate_Chain *> )
{
    int port = pref_port;
    SRef<Sip_Socket_Server *> server;

    SRef<Datagram_Socket *> sock = new UDP_Socket( port, ipv6 );
    server = new Datagram_Socket_Server( receiver, sock );
    server->set_external_ip( ip_string );

    pref_port = sock->get_port();
    return server;
}
