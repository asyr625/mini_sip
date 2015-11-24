#include<stdio.h>

#include "stun.h"
#include "stun_test.h"
#include "stun_message.h"
#include "stun_attribute.h"

#include "network_functions.h"

const int Stun::STUN_ERROR=0;
const int Stun::STUNTYPE_BLOCKED=1;
const int Stun::STUNTYPE_OPEN_INTERNET=2;
const int Stun::STUNTYPE_FULL_CONE=3;
const int Stun::STUNTYPE_SYMMETRIC_NAT=4;
const int Stun::STUNTYPE_PORT_RESTRICTED=5;
const int Stun::STUNTYPE_RESTRICTED=6;
const int Stun::STUNTYPE_SYMMETRIC_FIREWALL=7;

static const char *msgs[]={
    "ERROR",
    "BLOCKED",
    "OpenInternet",
    "FullCone",
    "SymmetricNAT",
    "PortRestricted",
    "Restricted",
    "SymmetricFirewall" };

int Stun::get_nat_type(IPAddress &stunAddr, uint16_t stunPort, UDP_Socket &socket, //IPAddress &localAddr,
                       std::vector<std::string> localIPs, uint16_t localPort)
{
    uint16_t dummy;
    return Stun::get_nat_type(stunAddr, stunPort, socket, localIPs, localPort, (char*)NULL, dummy);
}


void Stun::get_external_mapping(IPAddress &stunAddr, uint16_t stunPort, UDP_Socket &socket, char *bufferMappedIP,
                                uint16_t &mappedPort)
{
    //where message delete
    Stun_Message *message = Stun_Test::test(&stunAddr, stunPort, socket, false, false);

    Stun_Attribute_Mapped_Address *mappedAddr =
            (Stun_Attribute_Mapped_Address *)message->get_attribute(Stun_Attribute::MAPPED_ADDRESS);

    uint32_t firstTestIP = mappedAddr->get_binary_ip();

    mappedPort = mappedAddr->get_port();

    if (bufferMappedIP != NULL)
        Network_Functions::bin_ip_2_string(firstTestIP, bufferMappedIP);
}


int Stun::get_nat_type(IPAddress &stunAddr, uint16_t stunPort, UDP_Socket &socket, //IPAddress &localAddr,
                       std::vector<std::string> localIPs, uint16_t localPort, char *bufferMappedIP, uint16_t &mappedPort)
{
    Stun_Message *message;
    message = Stun_Test::test(&stunAddr, stunPort, socket, false, false);

    if( message == NULL )
        return STUNTYPE_BLOCKED;

    Stun_Attribute_Mapped_Address *mappedAddr =
            (Stun_Attribute_Mapped_Address *)message->get_attribute(Stun_Attribute::MAPPED_ADDRESS);

    uint32_t firstTestIP = mappedAddr->get_binary_ip();

    uint16_t firstTestPort = mappedAddr->get_port();

    Stun_Attribute_Mapped_Address *changedAddr =
            (Stun_Attribute_Mapped_Address *)message->get_attribute(Stun_Attribute::CHANGED_ADDRESS);

    uint32_t firstTestChangedIP = changedAddr->get_binary_ip();

    uint32_t firstTestChangedPort = changedAddr->get_port();

    if( bufferMappedIP != NULL )
        Network_Functions::bin_ip_2_string(firstTestIP, bufferMappedIP);
    mappedPort = firstTestPort;

    if (Network_Functions::is_local_ip(firstTestIP, localIPs)  &&  firstTestPort == localPort)
    {
        //cerr << "Same IP"<< endl;
        message = Stun_Test::test(&stunAddr, stunPort, socket, true, true);
        if( message == NULL)
        {
            return STUNTYPE_SYMMETRIC_FIREWALL;
        }
        else
        {
            return STUNTYPE_OPEN_INTERNET;
        }
    }
    else
    {
        //cerr << "testing restrictions (sym/restr/portrestr)"<< endl;
        char tmp[16];
        Network_Functions::bin_ip_2_string(firstTestChangedIP, tmp);
        //			IP4Address changedAddr(bin_ip_2_string(firstTestChangedIP));
        SRef<IPAddress*> changedAddr = IPAddress::create(tmp);


        message = Stun_Test::test(*changedAddr, firstTestChangedPort, socket, false, false);
        if( message == NULL )
        {
            return STUN_ERROR;
        }

        Stun_Attribute_Mapped_Address *mappedAddr =
                (Stun_Attribute_Mapped_Address *)message->get_attribute(Stun_Attribute::MAPPED_ADDRESS);
        if( mappedAddr->get_binary_ip() != firstTestIP &&
                mappedAddr->get_port() != firstTestPort )
        {
            //cerr << "TYPE: Symmetric"<< endl;
            return STUNTYPE_SYMMETRIC_NAT;
        }
        else
        {
            //cerr << "testing if restricted or port restrictd"<< endl;
            message = Stun_Test::test(&stunAddr, stunPort, socket, false, true);
            if (message == NULL)
            {
                //cerr << "TYPE: port restricted "<< endl;
                return STUNTYPE_PORT_RESTRICTED;
            }
            else
            {
                //cerr << "TYPE: restricted"<< endl;
                return STUNTYPE_RESTRICTED;
            }
        }
    }
    return 0;
}


const char *Stun::type_to_string(int t)
{
    return msgs[t];
}
