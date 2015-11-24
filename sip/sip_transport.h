#ifndef SIP_TRANSPORT_H
#define SIP_TRANSPORT_H

#include "splugin.h"
#include "sip_uri.h"
#include "ssingleton.h"
#include "stream_socket.h"

#include "sip_stack.h"
#include "sip_socket_server.h"

class Sip_Transport : public SPlugin
{
public:
    virtual std::string get_uri_scheme() const;
    virtual bool is_secure() const = 0;

    virtual std::string get_protocol() const = 0;
    virtual std::string get_via_protocol() const = 0;

    virtual int get_default_port() const;
    virtual std::string get_srv() const;

    virtual std::string get_naptr_service() const = 0;
    virtual int get_socket_type() const = 0;

    virtual SRef<Sip_Socket_Server *> create_server( SRef<Sip_Socket_Receiver*> receiver,
                                                  bool ipv6, const std::string &ip_string,
                                                  int &pref_port, SRef<Certificate_Set *> cert_db = NULL,
                                                  SRef<Certificate_Chain *> cert_chain = NULL ) = 0;

    virtual SRef<Stream_Socket *> connect( const IPAddress &addr, uint16_t port,
                                          SRef<Certificate_Set *> cert_db = NULL,
                                          SRef<Certificate_Chain *> cert_chain = NULL );

    std::string get_plugin_type() const { return "SipTransport"; }
protected:
    Sip_Transport( SRef<Library *> lib );
    Sip_Transport();
};


class Sip_Transport_Registry : public SPlugin_Registry, public SSingleton<Sip_Transport_Registry>
{
public:
    virtual std::string get_plugin_type(){ return "SipTransport"; }

    std::list<std::string> get_naptr_services( bool secure_only ) const;

    SRef<Sip_Transport*> find_transport( const std::string &protocol, bool secure=false ) const;

    SRef<Sip_Transport*> find_transport( const Sip_Uri &uri ) const;

    SRef<Sip_Transport*> find_via_transport( const std::string &protocol ) const;

    SRef<Sip_Transport*> find_transport( int socket_type ) const;

    SRef<Sip_Transport*> find_transport_by_name( const std::string &name ) const;
    SRef<Sip_Transport*> find_transport_by_naptr( const std::string &service ) const;

    std::list<SRef<Sip_Transport_Config*> > create_default_config() const;

protected:
    Sip_Transport_Registry();

private:
    friend class SSingleton<Sip_Transport_Registry>;
};

#endif // SIP_TRANSPORT_H
