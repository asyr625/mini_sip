#ifndef SIP_LAYER_TRANSPORT_H
#define SIP_LAYER_TRANSPORT_H

#include "my_types.h"

#include "cert.h"
#include "stream_socket.h"
#include "datagram_socket.h"
#include "sip_smcommand.h"
#include "sip_socket_server.h"

class Sip_Command_Dispatcher;
class Sip_Transport;
class Stream_Thread_Data;
class Stream_Thread_Server;

class Sip_Layer_Transport : public Sip_SMCommand_Receiver, public Sip_Socket_Receiver
{
public:
    Sip_Layer_Transport( SRef<Certificate_Chain *> cchain=NULL,
               SRef<Certificate_Set *> cert_db = NULL
    );

    virtual ~Sip_Layer_Transport();

    virtual void stop();

    bool handle_command(const Sip_SMCommand& command);

    void set_dispatcher(SRef<Sip_Command_Dispatcher*> d);

    virtual std::string get_mem_object_type() const {return "Sip_Layer_Transport";}

    void send_message(SRef<Sip_Message*> pack, const std::string &branch,
             bool addVia);

    void add_socket(SRef<Stream_Socket *> sock);
    void remove_socket(SRef<Stream_Socket *> sock);

    void add_server(SRef<Sip_Socket_Server *> server);

    SRef<Certificate_Chain *> get_certificate_chain();
    SRef<Certificate*> get_my_certificate();
    SRef<Certificate_Set *> get_certificate_set ();

    void datagram_socket_read(SRef<Datagram_Socket *> sock);

    void start_server( SRef<Sip_Transport*> transport, const std::string & ipString, const std::string & ip6String, int32_t &prefPort, int32_t externalUdpPort, SRef<Certificate_Chain *> certChain = NULL, SRef<Certificate_Set *> cert_db = NULL);

    void stop_server( SRef<Sip_Transport*> transport );

    int get_local_sip_port(const std::string &transport_name );

protected:

    void send_message(SRef<Sip_Message*> pack,
             const std::string &ip_addr,
            int32_t port,
            std::string branch,
            SRef<Sip_Transport*> transport,
            bool addVia
            );

    virtual SRef<Sip_Socket_Server *> find_server( int32_t type, bool ipv6);
    virtual SRef<Socket *> find_server_socket( int32_t type, bool ipv6);

private:
    bool validate_incoming(SRef<Sip_Message *> msg);

    bool get_destination(SRef<Sip_Message*> pack, std::string &dest_addr,
                int32_t &destPort, SRef<Sip_Transport*> &dest_transport);

    void add_via_header( SRef<Sip_Message*> pack, SRef<Sip_Socket_Server*> server, SRef<Socket *> socket, std::string branch );

    SRef<Stream_Socket *> find_stream_socket(IPAddress&address, uint16_t port);

    bool find_socket( SRef<Sip_Transport*> transport,
                 IPAddress &addr,
                 uint16_t port,
                 SRef<Sip_Socket_Server*> &server,
                 SRef<Socket*> &socket);

    void update_contact(SRef<Sip_Message*> pack,
               SRef<Sip_Socket_Server *> server,
               SRef<Socket *> socket);

    int _contact_udp_port;
    int _contact_sip_port;
    int _contact_sips_port;

    Mutex _servers_lock;
    std::list<SRef<Sip_Socket_Server *> > _servers;
    SRef<Socket_Server*> _manager;

    SRef<Certificate_Chain *> _cert_chain;
    SRef<Certificate_Set *> _cert_db;
    void * _tls_ctx;

    SRef<Sip_Command_Dispatcher*> _dispatcher;

    friend class Stream_Thread_Data;
};

void set_debug_print_packets(bool);
bool get_debug_print_packets();

#endif // SIP_LAYER_TRANSPORT_H
