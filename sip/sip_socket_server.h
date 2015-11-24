#ifndef SIP_SOCKET_SERVER_H
#define SIP_SOCKET_SERVER_H

#include "stream_socket.h"
#include "datagram_socket.h"
#include "socket_server.h"
#include "server_socket.h"

class Sip_Socket_Receiver : public virtual SObject {
public:
    virtual void add_socket(SRef<Stream_Socket *> sock) = 0;
    virtual void datagram_socket_read(SRef<Datagram_Socket *> sock) = 0;
};

class Sip_Socket_Server : public Socket_Server, public Input_Ready_Handler
{
public:
    Sip_Socket_Server(SRef<Sip_Socket_Receiver*> r, SRef<Socket*> sock );
    virtual ~Sip_Socket_Server();

    void free();
    std::string get_mem_object_type() const {return "SipSocketServer";}

    SRef<Socket *> get_socket() const;
    SRef<Sip_Socket_Receiver *> get_receiver() const;
    void set_receiver(SRef<Sip_Socket_Receiver *> r);

    bool is_ipv6() const;
    int get_type() const;

    const std::string &get_external_ip() const { return _external_ip; }
    void set_external_ip( const std::string &ip ) { _external_ip = ip; }

    void set_external_port(int port) { _external_port = port; }
    int get_external_port() const { return _external_port; }

    virtual void input_ready()=0;

protected:
    virtual void input_ready( SRef<Socket*> socket );
private:
    SRef<Sip_Socket_Receiver *> _receiver;
    SRef<Socket *> _ssock;

    std::string _external_ip;
    int _external_port;
};

class Stream_Socket_Server : public Sip_Socket_Server
{
public:
    Stream_Socket_Server(SRef<Sip_Socket_Receiver*> r, SRef<Server_Socket*> sock );
    std::string get_mem_object_type(){return "StreamSocketServer";}
    virtual void input_ready();
};

class Datagram_Socket_Server : public Sip_Socket_Server
{
public:
    Datagram_Socket_Server(SRef<Sip_Socket_Receiver*> r, SRef<Datagram_Socket*> sock );
    std::string get_mem_object_type(){return "DatagramSocketServer";}
    virtual void input_ready();
};

#endif // SIP_SOCKET_SERVER_H
