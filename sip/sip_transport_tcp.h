#ifndef SIP_TRANSPORT_TCP_H
#define SIP_TRANSPORT_TCP_H

#include "sip_transport.h"

class Sip_Transport_Tcp : public Sip_Transport
{
public:
    Sip_Transport_Tcp( SRef<Library *> lib );
    ~Sip_Transport_Tcp();

    virtual bool is_secure() const { return false; }

    virtual std::string get_protocol() const { return "tcp"; }
    virtual std::string get_via_protocol() const { return "TCP"; }

    virtual int get_socket_type() const { return SSOCKET_TYPE_TCP; }

    virtual std::string get_naptr_service() const { return "SIP+D2T"; }


    virtual SRef<Sip_Socket_Server *> create_server( SRef<Sip_Socket_Receiver*> receiver,
                                                  bool ipv6, const std::string &ip_string,
                                                  int &pref_port, SRef<Certificate_Set *> cert_db = NULL,
                                                  SRef<Certificate_Chain *> cert_chain = NULL );

    virtual SRef<Stream_Socket *> connect(const IPAddress &addr, uint16_t port,
                                          SRef<Certificate_Set *> = NULL,
                                          SRef<Certificate_Chain *> = NULL);

    virtual std::string get_name() const { return "TCP"; }

    virtual unsigned int get_version() const;

    virtual std::string get_description() const { return "SIP Transport TCP"; }

    virtual std::string get_mem_object_type() const { return get_name(); }
};

#endif // SIP_TRANSPORT_TCP_H
