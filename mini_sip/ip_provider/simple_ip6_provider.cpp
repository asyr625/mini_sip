#include "simple_ip6_provider.h"

#include "sip_configuration.h"
#include "network_functions.h"
#include "udp_socket.h"

#include <vector>
#include <algorithm>
#include <iostream>
using namespace std;

Simple_Ip6_Provider::Simple_Ip6_Provider( SRef<Sip_Configuration *> config )
{
    std::vector<SRef<Network_Interface*> > ifaces = Network_Functions::get_interfaces();
    bool use_ipv6 = true;
    Scope curScope = LINK_LOCAL;

    _local_ip = config->_sip_stack_config->local_ip_string;

    cout << "Simple_Ip6_Provider: localIp = " << _local_ip << endl;

    if (_local_ip.length()>0)
    {
        bool ok = false;
        for (unsigned i=0; i<ifaces.size(); i++)
        {
            SRef<Network_Interface *> iface = ifaces[i];
            vector<string> addrs = iface->get_ip_strings( true );
            vector<string>::iterator iter;

            cout << "Simple_Ip6_Provider: checking interface = " << iface->get_name() << endl;

            iter = find( addrs.begin(), addrs.end(), _local_ip );
            if ( iter != addrs.end() )
                ok = true;
        }
        if (!ok)
        {
            cerr << "Error: The IP address specified in the"
                    "configuration file ("<<_local_ip<<
                    ") is not configured on any local interface."<< endl;
            _local_ip = "";
        }
        else return;
    }

    for (unsigned i=0; i<ifaces.size(); i++)
    {
        void *ptr = &ifaces[i];

        my_dbg << "Simple_Ip6_Provider: checking ptr = " << ptr << endl;

        SRef<Network_Interface *> iface = ifaces[i];

        if( iface )
        {
            my_dbg << "Simple_Ip6_Provider: checking interface = " << *iface << endl;

            vector<string> addrs = iface->get_ip_strings( use_ipv6 );
            vector<string>::iterator iter;

            cout << "Simple_Ip6_Provider: checking interface = " << iface->get_name() << endl;

            for( iter = addrs.begin(); iter != addrs.end(); iter++ )
            {
                string ipstr = *iter;

                if ( ipstr  == string("::1") )
                {
                    if ( _local_ip.length() <= 0 )
                        _local_ip = ipstr;
                    continue;
                }

                Scope scope = ip_scope( ipstr );

                cout << "Simple_Ip6_Provider: checking interface = " << ifaces[i] << " with IP=" << ipstr << " scope=" << scope << endl;
                //only update the local ip if it is the first interface with a private
                //ip different from localhost or a public ip

                if( scope > curScope || _local_ip.empty() )
                {
                    _local_ip = ipstr;
                    curScope = scope;
                }
            }
        }
    }
    cout << "Simple_Ip6_Provider: using localIP =  " << _local_ip << endl;
}

Simple_Ip6_Provider::Scope Simple_Ip6_Provider::ip_scope( std::string ipstr )
{
    unsigned int prefix = strtol(ipstr.substr(0, 4).c_str(), NULL, 16);

    // Global IPv6 address:  |001|TLA|NLA|SLA|Interface

    // fec0::/10
    if( (prefix & 0xffc0) == 0xfec0 ){
        return SITE_LOCAL;
    }
    // fe80::/10
    else if( (prefix & 0xffc0) == 0xfe80 ){
        return LINK_LOCAL;
    }
    // fc00::/7
    else if( (prefix & 0xfe00) == 0xfc00 ){
        return UNIQUE_LOCAL;
    }
    // 2000::/3
    else if( (prefix & 0xe000) == 0x2000 ){
        return GLOBAL;
    }

    return INVALID;
}

std::string Simple_Ip6_Provider::get_external_ip()
{
    return _local_ip;
}

std::string Simple_Ip6_Provider::get_local_ip()
{
    return _local_ip;
}

void Simple_Ip6_Provider::set_external_ip(const std::string& _externalIP)
{
    _local_ip = _externalIP;
}

uint16_t Simple_Ip6_Provider::get_external_port( SRef<UDP_Socket *> sock )
{
    return sock->get_port();
}
