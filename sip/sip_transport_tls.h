#ifndef SIP_TRANSPORT_TLS_H
#define SIP_TRANSPORT_TLS_H

#include "sip_transport.h"

class Sip_Transport_Tls : public Sip_Transport
{
public:
    Sip_Transport_Tls();
    Sip_Transport_Tls( SRef<Library *> lib );
    ~Sip_Transport_Tls();

    virtual bool is_secure() const { return true; }

    virtual std::string get_protocol() const { return "tcp"; }
    virtual std::string get_via_protocol() const { return "TLS"; }

    virtual int get_socket_type() const { return SSOCKET_TYPE_TLS; }

    virtual std::string get_naptr_service() const { return "SIPS+D2T"; }


    virtual SRef<Sip_Socket_Server *> create_server( SRef<Sip_Socket_Receiver*> receiver,
                                                  bool ipv6, const std::string &ip_string,
                                                  int &pref_port, SRef<Certificate_Set *> cert_db = NULL,
                                                  SRef<Certificate_Chain *> cert_chain = NULL );

    virtual SRef<Stream_Socket *> connect( const IPAddress &addr, uint16_t port,
                                          SRef<Certificate_Set *> cert_db = NULL,
                                          SRef<Certificate_Chain *> cert_chain = NULL );

    virtual std::string get_name() const { return "TLS"; }

    virtual unsigned int get_version() const;

    virtual std::string get_description() const { return "SIP Transport TLS"; }

    virtual std::string get_mem_object_type() const { return get_name(); }
};

#endif // SIP_TRANSPORT_TLS_H
