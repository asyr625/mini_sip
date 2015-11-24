#include <iostream>
using namespace std;

#include "sip_socket_server.h"
#include "network_exception.h"

Sip_Socket_Server::Sip_Socket_Server(SRef<Sip_Socket_Receiver*> r, SRef<Socket*> sock ):
    _receiver(r), _ssock(sock)
{
    _external_port = _ssock->get_port();
    add_socket(sock, this);
}

Sip_Socket_Server::~Sip_Socket_Server()
{

}

void Sip_Socket_Server::free()
{
    remove_socket(_ssock);
}


bool Sip_Socket_Server::is_ipv6() const
{
    return _ssock->get_local_address()->get_type() == IP_ADDRESS_TYPE_V6;
}

int Sip_Socket_Server::get_type() const
{
    return _ssock->get_type();
}

SRef<Socket *> Sip_Socket_Server::get_socket() const
{
    return _ssock;
}

SRef<Sip_Socket_Receiver *> Sip_Socket_Server::get_receiver() const
{
    return _receiver;
}

void Sip_Socket_Server::set_receiver(SRef<Sip_Socket_Receiver *> r)
{
    _receiver = r;
}

void Sip_Socket_Server::input_ready( SRef<Socket*> socket )
{
    input_ready();
}

Stream_Socket_Server::Stream_Socket_Server(SRef<Sip_Socket_Receiver*> r, SRef<Server_Socket*> sock )
    : Sip_Socket_Server(r, *sock)
{
}

void Stream_Socket_Server::input_ready()
{
    SRef<Sip_Socket_Receiver*> r = get_receiver();
    SRef<Socket*> sock = get_socket();

    if( r && sock )
    {
        SRef<Server_Socket*> ssock = (Server_Socket*) *sock;
        SRef<Stream_Socket*> ss;

        try
        {
            ss = ssock->accept();
        } catch (Network_Exception &)
        {

        }

        if( ss )
        {
            r->add_socket(ss);
        }
        else
        {
            cerr << "Warning: Failed to accept client"<< endl;
        }
    }
}


Datagram_Socket_Server::Datagram_Socket_Server(SRef<Sip_Socket_Receiver*> r, SRef<Datagram_Socket*> sock )
    : Sip_Socket_Server(r, *sock)
{
}

void Datagram_Socket_Server::input_ready()
{
    SRef<Sip_Socket_Receiver*> transport = get_receiver();
    SRef<Socket*> sock = get_socket();

    if( transport && sock )
    {
        SRef<Datagram_Socket*> dsock = (Datagram_Socket*)*sock;
        transport->datagram_socket_read(dsock);
    }
}
