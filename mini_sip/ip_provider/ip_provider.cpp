#include "ip_provider.h"

#include "simple_ip_provider.h"
#include "stun_ip_provider.h"
#include "sip_configuration.h"
#include "simple_ip6_provider.h"


#include <string>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <iostream>
using namespace std;

#ifdef _WIN32_WCE
#	include "minisip_wce_extra_includes.h"
#endif


SRef<Ip_Provider *> Ip_Provider::create( SRef<Sip_Configuration *> config, bool useIpv6 )
{
    static bool isIPv6;
    static std::string localIP = obtain_local_ip_address(config, isIPv6);
    SRef<Ip_Provider *> ret;

    if( useIpv6 )
    {
        ret = new Simple_Ip6_Provider( config );
        if(!localIP.empty() || config->ipStack == Sip_Configuration::ipv4only)
        {
            if(!isIPv6 || config->ipStack == Sip_Configuration::ipv4only)
            {
                ret->set_external_ip("");
                cout << "Overriding local IPv6 address with \"\"" << std::endl;
            }
            else
            {
                ret->set_external_ip(localIP);
                cout << "Overriding local IPv6 address with \"" << localIP << "\"" << std::endl;
            }
        }
        return ret;
    }

    if(config->_use_stun)
        ret = Stun_Ip_Provider::create( config );

    if(!ret)
        ret = new Simple_Ip_Provider( config );

    if(!localIP.empty() || config->ipStack == Sip_Configuration::ipv6only)
    {
        if(isIPv6 || config->ipStack == Sip_Configuration::ipv6only)
        {
            ret->set_external_ip("");
            cout << "Overriding local IPv4 address with \"\"" << std::endl;
        }
        else
        {
            ret->set_external_ip(localIP);
            cout << "Overriding local IPv4 address with \"" << localIP << "\"" << std::endl;
        }
    }

    return ret;
}

std::string Ip_Provider::obtain_local_ip_address(SRef<Sip_Configuration *> config, bool& isIPv6)
{
    char localIP[INET6_ADDRSTRLEN] = "";
    if( config->ipStack == Sip_Configuration::autodetect )
    {
        cout << "Choosing suitable IP protocol to use... " << std::endl;
        if(!config || !config->_default_identity || !config->_default_identity->get_sip_registrar())
        {
            cerr << "Choosing suitable IP protocol to use failed, unable to obtain registrar uri!" << std::endl;
        }
        else
        {
            addrinfo *addressesList = NULL, resolvHint;
            memset(&resolvHint, 0, sizeof(resolvHint));
            resolvHint.ai_socktype = SOCK_DGRAM;

            unsigned short int remotePortNum = config->_default_identity->get_sip_registrar()->get_uri().get_port();
            if(remotePortNum == 0)
                remotePortNum = 5060;

            char remotePort[6];
            sprintf(remotePort, "%u", remotePortNum);
            cout << "Obtaining ip address of " << config->_default_identity->get_sip_registrar()->get_uri().get_ip().c_str()
                 << ", port " << remotePort << std::endl;

            if(getaddrinfo(config->_default_identity->get_sip_registrar()->get_uri().get_ip().c_str(), remotePort, &resolvHint, &addressesList) != 0)
            {
                cerr << "Choosing suitable IP protocol to use failed, unable to resolve registrar ip address!" << std::endl;
            }
            else
            {
                for(addrinfo *address = addressesList; address && strlen(localIP) <= 0; address = address->ai_next)
                {
                    cout << "Analysing addresses list element..." << std::endl;
                    bool protocolFamilyValid = true;
                    switch(address->ai_family)
                    {
                    case AF_INET:
                        cout << "using IPv4" << std::endl; break;
                    case AF_INET6:
                        cout << "using IPv6" << std::endl; break;
                    default:
                        cout << "unable to determine IP protocol!" << std::endl;
                        cerr << "Error: unable to determine IP protocol!" << std::endl;
                        protocolFamilyValid = false;
                        break;
                    }
                    if(protocolFamilyValid)
                    {
                        cout << "Obtaining suitable local address... " << std::endl;
                        int socketFd = 0;
                        if((socketFd = socket(address->ai_family, SOCK_DGRAM, 0)) == -1)
                        {
                            cerr << "Obtaining suitable local address failed, unable to create UDP socket! " << strerror(errno) << std::endl;
                        }
                        else
                        {
                            if(connect(socketFd, address->ai_addr, address->ai_addrlen) == -1)
                            {
                                cerr << "Obtaining suitable local address failed, unable to connect to sip registrar! " << strerror(errno) << std::endl;
                            }
                            else
                            {
                                sockaddr_in6 localAddress; // biggest of all socket address structs
                                socklen_t socketNameLength = (socklen_t) sizeof(localAddress);
                                if(getsockname(socketFd, (struct sockaddr *)&localAddress, &socketNameLength) == -1)
                                {
                                    cerr << "Obtaining suitable local address failed, unable to get socket local address! " << strerror(errno) << std::endl;
                                }
                                else
                                {
                                    if(address->ai_family == AF_INET)
                                    {
                                        isIPv6 = false;
                                        if(inet_ntop(address->ai_family, &((struct sockaddr_in&)localAddress).sin_addr.s_addr, localIP, sizeof(localIP)) == NULL)
                                            cout << "Socket name acquired, but inet_ntop failed" << std::endl;
                                        else
                                            cout << "Local IP address is: " << localIP << std::endl;
                                    }
                                    else if(address->ai_family == AF_INET6)
                                    {
                                        isIPv6 = true;
                                        if(inet_ntop(address->ai_family, &localAddress.sin6_addr, localIP, sizeof(localIP)) == NULL)
                                            cout << "Socket name acquired, but inet_ntop failed" << std::endl;
                                        else
                                            cout << "Local IP address is: " << localIP << std::endl;
                                    }
                                }
                            }
                        }
                        close(socketFd);
                    }
                }
            }
            freeaddrinfo(addressesList);
        }
    }
}
