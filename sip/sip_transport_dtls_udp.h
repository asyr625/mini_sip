#ifndef SIP_TRANSPORT_DTLS_UDP_H
#define SIP_TRANSPORT_DTLS_UDP_H

#include "dtls_socket.h"
#include "sip_transport.h"

class Sip_Transport_Dtls_Udp : public Sip_Transport
{
public:
    Sip_Transport_Dtls_Udp( SRef<Library *> lib );
    ~Sip_Transport_Dtls_Udp();

    virtual bool is_secure() const { return true; }

    virtual std::string get_protocol() const { return "udp"; }
    virtual std::string get_via_protocol() const { return "DTLS-UDP"; }

    virtual int get_socket_type() const { return SSOCKET_TYPE_DTLS_UDP; }

    virtual std::string get_naptr_service() const { return "SIPS+D2U"; }


    virtual SRef<Sip_Socket_Server *> create_server( SRef<Sip_Socket_Receiver*> receiver,
                                                  bool ipv6, const std::string &ip_string,
                                                  int &pref_port, SRef<Certificate_Set *> cert_db = NULL,
                                                  SRef<Certificate_Chain *> cert_chain = NULL );

    virtual std::string get_name() const { return "DTLS-UDP"; }

    virtual unsigned int get_version() const;

    virtual std::string get_description() const { return "SIP Transport DTLS-UDP"; }

    virtual std::string get_mem_object_type() const { return get_name(); }
};

#endif // SIP_TRANSPORT_DTLS_UDP_H
