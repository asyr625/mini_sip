#include <algorithm>
#include <bits/algorithmfwd.h>

#include "sip_transport.h"

#include "sip_transport_udp.h"
#include "sip_transport_tcp.h"
#include "sip_transport_tls.h"

#include "sip_transport_sctp.h"
#include "sip_transport_dtls_udp.h"
#include "sip_transport_tls_sctp.h"

Sip_Transport::Sip_Transport() : SPlugin()
{
}

Sip_Transport::Sip_Transport( SRef<Library *> lib )
    : SPlugin(lib)
{

}
std::string Sip_Transport::get_uri_scheme() const
{
    return is_secure() ? "sips" : "sip";
}

int Sip_Transport::get_default_port() const
{
    return is_secure() ? 5061 : 5060;
}
std::string Sip_Transport::get_srv() const
{
    const std::string &service = get_uri_scheme();
    const std::string &proto = get_protocol();

    return "_" + service + "._" + proto;
}

SRef<Stream_Socket *> Sip_Transport::connect(const IPAddress &, uint16_t,
                                          SRef<Certificate_Set *> ,
                                          SRef<Certificate_Chain *> )
{
    throw Exception("Connection less transport");
}


Sip_Transport_Registry::Sip_Transport_Registry()
{
    register_plugin( new Sip_Transport_Udp( NULL ) );
    register_plugin( new Sip_Transport_Tcp( NULL ) );
    register_plugin( new Sip_Transport_Tls( NULL ) );
#ifdef HAVE_DTLS
    register_plugin( new Sip_Transport_Dtls_Udp( NULL ) );
#endif
#ifdef HAVE_SCTP
    register_plugin( new Sip_Transport_Sctp( NULL ) );
    register_plugin( new Sip_Transport_Tls_Sctp( NULL ) );
#endif
}

std::list<std::string> Sip_Transport_Registry::get_naptr_services( bool secure_only ) const
{
    std::list<std::string> services;
    std::list< SRef<SPlugin*> >::const_iterator iter;
    std::list< SRef<SPlugin*> >::const_iterator stop = _plugins.end();

    for( iter = _plugins.begin(); iter != stop; iter++ )
    {
        SRef<SPlugin*> plugin = *iter;

        if( !plugin )
            continue;

        SRef<Sip_Transport*> transport =
            dynamic_cast<Sip_Transport*>( *plugin );

        if( !transport )
            continue;

        if( !secure_only || transport->is_secure() )
        {
            services.push_back( transport->get_naptr_service() );
        }
    }

    return services;
}

SRef<Sip_Transport*> Sip_Transport_Registry::find_transport( const std::string &protocol, bool secure ) const
{
    std::string lc_port = protocol;
    std::list< SRef<SPlugin*> >::const_iterator iter;
    std::list< SRef<SPlugin*> >::const_iterator stop = _plugins.end();

    transform(lc_port.begin(), lc_port.end(),
              lc_port.begin(), (int(*)(int))tolower);

    for( iter = _plugins.begin(); iter != stop; iter++ )
    {
        SRef<SPlugin*> plugin = *iter;

        if( !plugin )
            continue;

        SRef<Sip_Transport*> transport =
            dynamic_cast<Sip_Transport*>( *plugin );

        if( !transport )
            continue;
        if( transport->get_protocol() == lc_port && transport->is_secure() == secure )
        {
            return transport;
        }
    }
    return NULL;
}

SRef<Sip_Transport*> Sip_Transport_Registry::find_transport( const Sip_Uri &uri ) const
{
    return find_transport( uri.get_transport(), uri.get_protocol_id() == "sips" );
}

SRef<Sip_Transport*> Sip_Transport_Registry::find_via_transport( const std::string &protocol ) const
{
    std::string uc_port = protocol;

    std::list< SRef<SPlugin*> >::const_iterator iter;
    std::list< SRef<SPlugin*> >::const_iterator stop = _plugins.end();

    transform(uc_port.begin(), uc_port.end(),
              uc_port.begin(), (int(*)(int))toupper);

    for( iter = _plugins.begin(); iter != stop; iter++ )
    {
        SRef<SPlugin*> plugin = *iter;

        if( !plugin )
            continue;

        SRef<Sip_Transport*> transport =
            dynamic_cast<Sip_Transport*>( *plugin );

        if( !transport )
            continue;
        if( transport->get_via_protocol() == uc_port )
        {
            return transport;
        }
    }
    return NULL;
}

SRef<Sip_Transport*> Sip_Transport_Registry::find_transport( int socket_type ) const
{
    std::list< SRef<SPlugin*> >::const_iterator iter;
    std::list< SRef<SPlugin*> >::const_iterator stop = _plugins.end();
    for( iter = _plugins.begin(); iter != stop; iter++ )
    {
        SRef<SPlugin*> plugin = *iter;

        if( !plugin )
            continue;

        SRef<Sip_Transport*> transport =
            dynamic_cast<Sip_Transport*>( *plugin );

        if( !transport )
            continue;
        if( transport->get_socket_type() == socket_type )
        {
            return transport;
        }
    }
    return NULL;
}

SRef<Sip_Transport*> Sip_Transport_Registry::find_transport_by_name( const std::string &name ) const
{
    SRef<SPlugin*> transport = find_plugin( name );

    if( transport )
    {
        return dynamic_cast<Sip_Transport*>(*transport);
    }

    return NULL;
}
SRef<Sip_Transport*> Sip_Transport_Registry::find_transport_by_naptr( const std::string &service ) const
{
    std::list< SRef<SPlugin*> >::const_iterator iter;
    std::list< SRef<SPlugin*> >::const_iterator stop = _plugins.end();
    for( iter = _plugins.begin(); iter != stop; iter++ )
    {
        SRef<SPlugin*> plugin = *iter;

        if( !plugin )
            continue;

        SRef<Sip_Transport*> transport =
            dynamic_cast<Sip_Transport*>( *plugin );

        if( !transport )
            continue;
        if( transport->get_naptr_service() == service )
        {
            return transport;
        }
    }
    return NULL;
}

std::list<SRef<Sip_Transport_Config*> > Sip_Transport_Registry::create_default_config() const
{
    std::list< SRef<Sip_Transport_Config* > > result;

    std::list< SRef<SPlugin*> >::const_iterator iter;
    std::list< SRef<SPlugin*> >::const_iterator stop = _plugins.end();
    for( iter = _plugins.begin(); iter != stop; iter++ )
    {
        SRef<SPlugin*> plugin = *iter;

        if( !plugin )
            continue;

        SRef<Sip_Transport*> transport =
            dynamic_cast<Sip_Transport*>( *plugin );

        if( !transport )
            continue;
        SRef<Sip_Transport_Config* > config =
            new Sip_Transport_Config( transport->get_name() );

        result.push_back( config );
    }
    return result;
}
