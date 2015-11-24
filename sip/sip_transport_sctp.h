#ifndef SIP_TRANSPORT_SCTP_H
#define SIP_TRANSPORT_SCTP_H

#include "sip_transport.h"

class Sip_Transport_Sctp : public Sip_Transport
{
public:
    Sip_Transport_Sctp( SRef<Library *> lib );
    ~Sip_Transport_Sctp();

    virtual bool is_secure() const { return false; }

    virtual std::string get_protocol() const { return "sctp"; }
    virtual std::string get_via_protocol() const { return "SCTP"; }

    virtual int get_socket_type() const { return SSOCKET_TYPE_SCTP; }

    virtual std::string get_naptr_service() const { return "SIP+D2S"; }


    virtual SRef<Sip_Socket_Server *> create_server( SRef<Sip_Socket_Receiver*> receiver,
                                                  bool ipv6, const std::string &ip_string,
                                                  int &pref_port, SRef<Certificate_Set *> cert_db = NULL,
                                                  SRef<Certificate_Chain *> cert_chain = NULL );

    virtual SRef<Stream_Socket *> connect( const IPAddress &addr, uint16_t port,
                                          SRef<Certificate_Set *> cert_db = NULL,
                                          SRef<Certificate_Chain *> cert_chain = NULL );

    virtual std::string get_name() const { return "SCTP"; }

    virtual unsigned int get_version() const;

    virtual std::string get_description() const { return "SIP Transport SCTP"; }

    virtual std::string get_mem_object_type() const { return get_name(); }
};

#endif // SIP_TRANSPORT_SCTP_H
