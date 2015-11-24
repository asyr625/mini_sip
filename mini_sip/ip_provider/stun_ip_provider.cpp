#include "stun_ip_provider.h"

#include "sip_configuration.h"
#include "udp_socket.h"
#include "network_exception.h"
#include "network_functions.h"
#include "stun.h"

#include<stdlib.h>

#ifdef _WIN32_WCE
#	include "minisip_wce_extra_includes.h"
#endif
#include <iostream>
using namespace std;


static vector<string> getLocalIPs()
{
    std::vector<string> ret;
    std::vector<string> ifaces = Network_Functions::get_all_interfaces();
    for (unsigned i=0; i<ifaces.size(); i++)
    {
        string ip = Network_Functions::get_interface_ipstr(ifaces[i]);
        my_dbg << "Adding local ip: "<< ip <<  endl;
        ret.push_back(ip);
    }
    return ret;
}

static std::string findStunServer( SRef<Sip_Configuration *> phoneConf, uint16_t stunPort )
{

#ifdef DEBUG_OUTPUT
    my_dbg << "Try 1, autodetect"<< endl;
#endif
    if (phoneConf->_find_stun_server_from_sip_uri)
    {
        my_dbg << "Using SIP uri: "<<phoneConf->_default_identity->get_sip_uri().get_string()<< endl;
        const Sip_Uri &useruri = phoneConf->_default_identity->get_sip_uri();
        const string &uridomain = useruri.get_ip();
        my_dbg << "domain=<" << uridomain << ">" << endl;

        if (uridomain.length()>0)
        {
            uint16_t port;
            string proxy = Network_Functions::get_host_handling_service("_stun._udp",uridomain, port);
            if (proxy.length()>0)
            {
                phoneConf->_stun_server_ip_string = proxy;
                phoneConf->_stun_server_port = (uint16_t)port;
                return proxy;
            }
        }
    }

#ifdef DEBUG_OUTPUT
    my_out << "Try 2, checkig if configured to use domain"<< endl;
#endif
    if (phoneConf->_find_stun_server_from_domain && phoneConf->_stun_domain.length()>0)
    {
        uint16_t port;
        string proxy = Network_Functions::get_host_handling_service( "_stun._udp",phoneConf->_stun_domain, port);
        if (proxy.length()>0)
        {
            phoneConf->_stun_server_ip_string = proxy;
            phoneConf->_stun_server_port = (uint16_t)port;
            return proxy;
        }
    }
#ifdef DEBUG_OUTPUT
    my_out << "Try 3, checking if user defined"<< endl;
#endif
    if (phoneConf->_use_user_defined_stun_server && phoneConf->_user_defined_stun_server.length() > 0)
    {
        uint16_t port=3478;
        string addr = phoneConf->_user_defined_stun_server;
        if (addr.find(":")!=string::npos)
        {
            string portstr = addr.substr(addr.find(":")+1);
            addr = addr.substr(0,addr.find(":"));
            port = atoi(portstr.c_str());
        }
        phoneConf->_stun_server_ip_string = addr;
        phoneConf->_stun_server_port = port;
        return addr;
    }
    return "";
}

SRef<Ip_Provider *> Stun_Ip_Provider::create( SRef<Sip_Configuration *> phoneConf )
{
    std::vector<string> localips = getLocalIPs();

    SRef<IPAddress *> stunIp = NULL;
    bool done = false;

    do{
        done = true;
        uint16_t port = 0;
        string proxy = findStunServer(phoneConf, port);

        try
        {
            stunIp = IPAddress::create(phoneConf->_stun_server_ip_string, false);
        }
        catch(Host_Not_Found & )
        {
            my_err << "Could not find your STUN server. "
                    "STUN will be disabled." << endl;
            return NULL;
            done = false;
        }

        if (!stunIp)
            return NULL;

        if( !phoneConf->_use_stun )
        {
            /* The user no longer wants to use STUN */
            return NULL;
        }

    } while( !done );

    uint16_t stunPort = phoneConf->_stun_server_port;

    UDP_Socket sock;

    uint16_t localPort = (uint16_t)sock.get_port();
    char mappedip[16];
    uint16_t mappedport;
    int32_t natType = Stun::get_nat_type( **stunIp, stunPort,
                                        sock, localips, localPort, mappedip, mappedport );

    if( natType == Stun::STUN_ERROR )
    {
        my_err << "An error occured while minisip tried to "
                "discover the NAT type with STUN. "
                "STUN support will be disabled." << endl;
        return NULL;
    }

    if( natType == Stun::STUNTYPE_BLOCKED )
    {
        my_err << "minisip could not contact your STUN server. "
                "STUN support will be disabled." << endl;
        return NULL;
    }

    string externalIp = mappedip;
#ifdef DEBUG_OUTPUT
    mout << "NAT type is: " << Stun::type_to_string( natType ) <<
            " and the external contact IP is set to "<<
            externalIp << endl;
#endif

    return (Ip_Provider*) new Stun_Ip_Provider( natType, externalIp, stunIp, stunPort  );
}

Stun_Ip_Provider::Stun_Ip_Provider( uint32_t natType, std::string externalIp, SRef<IPAddress *> stunIp, uint16_t stunPort )
    : _stun_ip(stunIp),
      _stun_port(stunPort),
      _external_ip(externalIp),
      _nat_type(natType)
{
}

Stun_Ip_Provider::~Stun_Ip_Provider()
{
}

std::string Stun_Ip_Provider::get_external_ip()
{
    return _external_ip;
}

void Stun_Ip_Provider::set_external_ip(const std::string& external_ip)
{
    _external_ip = external_ip;
}

uint16_t Stun_Ip_Provider::get_external_port( SRef<UDP_Socket *> socket )
{
    char mappedIPBuffer[16];
    uint16_t mappedPort;

    if( _nat_type == (unsigned)Stun::STUNTYPE_OPEN_INTERNET )
    {
        /* In that case, don't bother do the STUN query */
        return (uint16_t)socket->get_port();
    }

    Stun::get_external_mapping( **_stun_ip, _stun_port, **socket, mappedIPBuffer, mappedPort );

    if( string( mappedIPBuffer ) != _external_ip )
    {
        _external_ip = string( mappedIPBuffer );
    }
    return mappedPort;
}
