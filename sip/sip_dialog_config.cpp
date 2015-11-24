#include "sip_dialog_config.h"

#include "network_exception.h"
#include "network_functions.h"
#include "my_assert.h"
#include "string_utils.h"
#include "sip_transport.h"

#include <cstdlib>
#include<stdlib.h>

using namespace std;

Sip_Credential::Sip_Credential( const std::string &username, const std::string &password, const std::string &realm )
    : _username(username),
      _password(password),
      _realm(realm)
{
}

const std::string &Sip_Credential::get_realm() const
{
    return _realm;
}
const std::string &Sip_Credential::get_username() const
{
    return _username;
}
const std::string &Sip_Credential::get_password() const
{
    return _password;
}

void Sip_Credential::set( const std::string &username, const std::string &password, const std::string &realm )
{
    _username = username;
    _password = password;
    _realm = realm;
}


Sip_Registrar::Sip_Registrar()
    : _uri()
{
    auto_detect_settings = false;
    _register_expires = DEFAULT_SIPPROXY_EXPIRES_VALUE_SECONDS;
    _default_expires = DEFAULT_SIPPROXY_EXPIRES_VALUE_SECONDS;
}

Sip_Registrar::Sip_Registrar(const Sip_Uri &addr, int port)
{
    auto_detect_settings = false;
    try {
        _register_expires = DEFAULT_SIPPROXY_EXPIRES_VALUE_SECONDS;
        _default_expires = DEFAULT_SIPPROXY_EXPIRES_VALUE_SECONDS;
        set_registrar( addr, port );
    } catch (Network_Exception & )
    {
#ifdef DEBUG_OUTPUT
        cerr << "SipRegistrar(str, int) throwing ... " << endl;
#endif
        throw Host_Not_Found( "[SipRegistrar " + addr.get_string() + "]" );
    }
}

Sip_Registrar::Sip_Registrar(const Sip_Uri &userUri, std::string transportParam)
{
    Sip_Uri addr;
    auto_detect_settings = true;

    _register_expires = DEFAULT_SIPPROXY_EXPIRES_VALUE_SECONDS;
    _default_expires = DEFAULT_SIPPROXY_EXPIRES_VALUE_SECONDS;

    addr = userUri;
    if( transportParam != "" )
        addr.set_transport( transportParam );

    set_registrar( addr );
}

std::string Sip_Registrar::get_debug_string()
{
    return "uri="+_uri.get_string()
            +"; autodetect="+ (auto_detect_settings?"yes":"no")
            // 		+"; user="+sipProxyUsername
            // 		+"; password="+sipProxyPassword
            +"; expires="+itoa(_default_expires);
}

void Sip_Registrar::set_register_expires( std::string _expires )
{
    int r;
    r = atoi( _expires.c_str() );
    set_register_expires( r );
}

void Sip_Registrar::set_register_expires( int _expires )
{
    if( _expires >= 0 && _expires < 100000 ) //sanity check ...
        _register_expires = _expires;
    else _register_expires = DEFAULT_SIPPROXY_EXPIRES_VALUE_SECONDS;
}

std::string Sip_Registrar::get_register_expires()
{
    return itoa(_register_expires);
}

int Sip_Registrar::get_register_expires_int( )
{
    return _register_expires;
}

void Sip_Registrar::set_default_expires( std::string _expires )
{
    int r;
    r = atoi( _expires.c_str() );
    set_default_expires( r );
}

void Sip_Registrar::set_default_expires( int _expires )
{
    if( _expires >= 0 && _expires < 100000 ) //sanity check ...
        _default_expires = _expires;
    else _default_expires = DEFAULT_SIPPROXY_EXPIRES_VALUE_SECONDS;
}

std::string Sip_Registrar::get_default_expires()
{
    return itoa(_default_expires);
}

int Sip_Registrar::get_default_expires_int()
{
    return _default_expires;
}


void Sip_Identity::init()
{
    _register_to_proxy = false;
    security_enabled = false;
    ka_type=0;
    //use_srtp=false;
    use_zrtp = false;
    psk_enabled = false;
    dh_enabled = false;
    check_cert = false;

    identity_idx = itoa( global_index );
    global_index ++;
#ifdef DEBUG_OUTPUT
    my_dbg << "Sip_Identity::SipIdentity : created identity id=" << identity_idx << endl;
#endif
    set_is_registered (false);
}

Sip_Identity::Sip_Identity()
{
    init();
}

Sip_Identity::Sip_Identity(const Sip_Uri &sipuri) : _sip_uri(sipuri)
{
    init();
}

void Sip_Identity::set_do_register(bool f)
{
    lock();
    _register_to_proxy = f;
    unlock();
}

bool Sip_Identity::get_do_register()
{
    lock();
    bool ret = _register_to_proxy;
    unlock();
    return ret;
}

void Sip_Identity::lock()
{
    mutex.lock();
}

void Sip_Identity::unlock()
{
    mutex.unlock();
}

std::string Sip_Identity::get_id()
{
    lock();
    std::string ret = identity_idx;
    unlock();
    return ret;
}

bool Sip_Identity::is_registered()
{
    lock();
    bool ret = currently_registered;
    unlock();
    return ret;
}

SRef<Sip_Registrar *> Sip_Identity::get_sip_registrar()
{
    return sip_proxy;
}

bool Sip_Identity::set_sip_registrar( SRef<Sip_Registrar *> proxy )
{
    sip_proxy = proxy;
    return true;
}

SRef<Sip_Credential*> Sip_Identity::get_credential() const
{
    return credential;
}
void Sip_Identity::set_credential( SRef<Sip_Credential*> aCredential )
{
    credential = aCredential;
}

const std::list<Sip_Uri> &Sip_Identity::get_route_set() const
{
    return route_set;
}
void Sip_Identity::set_route_set( const std::list<Sip_Uri> &routeSet )
{
    route_set = routeSet;
}

void Sip_Identity::add_route( const Sip_Uri &route )
{
    route_set.push_back( route );
}

void Sip_Identity::set_identity_name(std::string n)
{
    identity_identifier = n;
}

void Sip_Identity::set_is_registered( bool registerOk )
{
    if( registerOk == true && get_sip_registrar()->get_register_expires_int() != 0 )
    {
        currently_registered = true;
    } else {
        currently_registered = false;
    }
}

void Sip_Identity::set_registered_contacts( const std::list<Sip_Uri> &contacts )
{
    registered_contacts = contacts;
}

const std::list<Sip_Uri>& Sip_Identity::get_registered_contacts() const
{
    return registered_contacts;
}

std::string Sip_Identity::set_sip_proxy( bool autodetect, std::string userUri, std::string transportName, std::string proxyAddr, int proxyPort )
{
    string ret = "";
    SRef<Sip_Transport*> transport;
    Sip_Uri proxyUri;
    bool useProxy = false;

    if( !transportName.empty() )
    {
        transport = Sip_Transport_Registry::get_instance()->find_transport_by_name( transportName );
    }

    route_set.clear();

#ifdef DEBUG_OUTPUT
    if( autodetect ) my_dbg << "Sip_Identity::set_sip_proxy: autodetect is true";
    else 		my_dbg<< "Sip_Identity::set_sip_proxy: autodetect is false";
    my_dbg << "; userUri=" << userUri << "; transport = "<< transportName << "; proxyAddr=" << proxyAddr << "; proxyPort=" << proxyPort << endl;
#endif

    if( autodetect )
    {
        Sip_Uri aor( userUri );

        proxyUri.set_protocol_id( aor.get_protocol_id() );
        proxyUri.set_ip( aor.get_ip() );
        useProxy = true;
    }
    else if( proxyAddr != "" )
    {
        proxyUri.set_protocol_id( "sip" );
        proxyUri.set_ip( proxyAddr );
        proxyUri.set_port( proxyPort );
        useProxy = true;
    }

    if( useProxy )
    {
        if( transport )
        {
            proxyUri.set_protocol_id( transport->get_uri_scheme() );
            proxyUri.set_transport( transport->get_protocol() );
        }

        proxyUri.set_parameter( "lr", "true" );
        proxyUri.make_valid( true );

        route_set.push_back( proxyUri );
    }
    return ret;
}


std::string Sip_Identity::get_debug_string()
{
    lock();
    string ret = "identity="+identity_idx+
            "; uri="+_sip_uri.get_string()+
            " proxy=["+(get_sip_registrar()?get_sip_registrar()->get_debug_string():"")+
            "]; isRegistered="+itoa(currently_registered);
    unlock();
    return ret;
}

void Sip_Identity::set_psk( std::string key )
{
    psk = key;
}

Sip_Uri Sip_Identity::get_contact_uri( SRef<Sip_Stack*> sipStack, bool useStun ) const
{
    const Sip_Identity * sipIdentity = this;
    Sip_Uri contactUri;
    const Sip_Uri &fromUri = sipIdentity->get_sip_uri();
    string transport;
    int port = 0;

    const list<Sip_Uri> &routes = sipIdentity->get_route_set();

    if( !routes.empty() )
    {
        Sip_Uri proxy = *routes.begin();

        transport = proxy.get_transport();
    }
    else
        transport = fromUri.get_transport();

    port =sipStack->get_local_sip_port(useStun, transport);

    contactUri.set_params( fromUri.get_user_name(),
                           sipStack->get_stack_config()->external_contact_ip,
                           "",
                           port);
    if( transport != "" )
        contactUri.set_transport( transport );

    contactUri.set_parameter("minisip", "true");

    return contactUri;
}


Sip_Dialog_Config::Sip_Dialog_Config(SRef<Sip_Stack *> stack)
    : _sip_stack (stack)
{
    _local_ssrc = rand();
}

void Sip_Dialog_Config::use_identity( SRef<Sip_Identity*> identity, std::string transport)
{
    _sip_identity = identity;
}

Sip_Uri Sip_Dialog_Config::get_contact_uri( bool use_stun ) const
{
    return _sip_identity->get_contact_uri( _sip_stack, use_stun );
}
