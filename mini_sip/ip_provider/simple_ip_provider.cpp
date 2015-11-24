#include <vector>
#include "simple_ip_provider.h"

#include "sip_configuration.h"
#include "network_exception.h"
#include "network_functions.h"
#include "udp_socket.h"

#ifdef _WIN32_WCE
#	include "minisip_wce_extra_includes.h"
#endif

#include <iostream>
using namespace std;

Simple_Ip_Provider::Simple_Ip_Provider( SRef<Sip_Configuration *> config )
{
    unsigned i; //index
    std::vector<string> ifaces = Network_Functions::get_all_interfaces();

    _local_ip = config->_sip_stack_config->local_ip_string;
#ifdef DEBUG_OUTPUT
    cerr << "Simple_Ip_Provider: _local_ip = " << _local_ip << endl;
#endif

    if (_local_ip.length()>0)
    {
        bool ok=false;
        for ( i = 0; i < ifaces.size(); i++ )
        {
            if( _local_ip == Network_Functions::get_interface_ipstr(ifaces[i]))
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

    bool ipFound = false;

    //if a preferred network interface is specified in the config file ...
    if( config->_network_interface_name != "")
    {
        for ( i = 0; i < ifaces.size(); i++)
        {
            if ( config->_network_interface_name == ifaces[i] )
            {
                _local_ip = Network_Functions::get_interface_ipstr(ifaces[i]);
                ipFound = true;
                break;
            }
        }


#ifdef DEBUG_OUTPUT
        cerr << "SimpleIPProvider: preferred network interface = " << config->_network_interface_name  << endl;
        if( ipFound ) cerr << "SimpleIPProvider: preferred interface found" << endl;
        else cerr << "SimpleIPProvider: preferred interface NOT found" << endl;
        if ( ipFound && _local_ip=="" ){
            cerr << "SimpleIPProvider: WARNING: prefered interface has no IP address configured"<<endl;
        }
#endif
        //If the preferred interface is without IP, continue searching...
        if (_local_ip == "")
            ipFound = false;
    }

    //if ip is not found (either not specified or the adapter is not good ...
    //use one which we consider apropriate
    if( ! ipFound )
    {
        //print message telling the user about defining a preferred interface
        cout <<    "===========================Simple_Ip_Provider=============================" << endl
                << "|No network interface defined as preferred in the configuration, or" << endl
                << "|the one specified could not be found." << endl
                << "|Minisip will try to find an appropriate one." << endl
                << "|Minisip highly recommends you to add a preferred one. To do so, choose" << endl
                << "|    from the list below and edit the configuration file, section <network_interface>" << endl
                << "|    or use the GUI configuration;" << endl;
        for( i = 0; i < ifaces.size(); i++ )
        {
            string ip = Network_Functions::get_interface_ipstr(ifaces[i]);
            cout << "|       Network Interface: name = " << ifaces[i] << "; IP=" << ip << endl;
        }
        cout <<    "========================================================================" << endl;
        for ( i = 0; i < ifaces.size(); i++ )
        {
            string ip = Network_Functions::get_interface_ipstr(ifaces[i]);
#ifdef DEBUG_OUTPUT
            //cout << "Simple_IP_Provider: interface = " << ifaces[i] << "; IP=" << ip << endl;
#endif
            if (ip.length()>0)
            {
                if (ifaces[i]==string("lo"))
                { //this interface only exhists in linux ...
                    if (_local_ip.length() <= 0)
                        _local_ip = ip;
                }
                else
                {
                    string ipstr = ip;

                    //only update the local ip if it is the first interface with a private
                    //ip different from localhost or a public ip
                    if ( is_in_private_ip_range( ipstr ))
                    {
                        if (_local_ip.length()<=0 || _local_ip == "127.0.0.1" || //this is the lo interface
                                _local_ip.substr(0,2)=="0."  //0.0.0.0 is used by windows ...
                                )
                            _local_ip = ipstr;
                    }
                    else
                    {
                        //use first public ip we find ... overwritting the private one
                        if( _local_ip.length() <= 0 || _local_ip=="127.0.0.1" || _local_ip.substr(0,2)=="0." ||is_in_private_ip_range( _local_ip) )
                            _local_ip = ipstr;
                    }
                }
            }
        }
    }
    cout << "SimpleIpProvider is using local IP =  " << _local_ip << endl;
}

bool Simple_Ip_Provider::is_in_private_ip_range( std::string ipstr )
{
    //check the easy ones first ... 10.x.x.x, 127.x.x.x,
    //192.168.x.x and 0.x.x.x
    if (ipstr.substr(0,3)=="10." || ipstr.substr(0,4)=="127." || ipstr.substr(0,7)=="192.168" || ipstr.substr(0,2)=="0.")
    {	//Found local interfaces in Windows XP used to communicate only
        //internally with a web camera that started with "0."
        return true;
    }
    //this range goes from 172.16.x.x to 172.31.x.x
    if( ipstr.substr(0,4)=="172." )
    {
        if( ipstr[6] == '.' )
        {
            if( ipstr[4] == '1' )
            {
                if( ipstr[5] == '6' || ipstr[5] == '7' || ipstr[5] == '8' || ipstr[5] == '9' )
                {
                    return true;
                }
            }
            else if( ipstr[4] == '2' ) { return true; }
            else if( ipstr[4] == '3' && ipstr[5] == '1' ) { return true;}
        }
    }

    //finally, check for automatic ip private addresses (used by mocosoft)
    if( ipstr.substr(0,7)=="169.254" )
    {
        return true;
    }

    return false;
}

std::string Simple_Ip_Provider::get_external_ip()
{
    return _local_ip;
}

std::string Simple_Ip_Provider::get_local_ip()
{
    return _local_ip;
}

uint16_t Simple_Ip_Provider::get_external_port( SRef<UDP_Socket *> sock )
{
    return (uint16_t)sock->get_port();
}

void Simple_Ip_Provider::set_external_ip(const std::string& _external_ip)
{
    _local_ip = _external_ip;
}
