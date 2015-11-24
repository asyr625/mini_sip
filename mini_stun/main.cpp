#include <iostream>
using namespace std;

#include "ipaddress.h"
#include "udp_socket.h"
#include "network_functions.h"
#include "timeout_provider.h"
#include "stun.h"

using namespace std;

int main(int argc, char **argv)
{
    SRef<IPAddress *> addr =
        IPAddress::create("stun.fwdnet.net", false);
    unsigned short stunPort = 3478;
    UDP_Socket sock;

    vector<string> interfaces = Network_Functions::get_all_interfaces();
    vector<string> localAddr;
    for (uint32_t i; i < interfaces.size(); i++)
    {
        string &iface = interfaces[i];
        cerr << "i" << i << ": " << iface << endl;
        localAddr[i] = Network_Functions::get_interface_ipstr(iface);
    }

    unsigned short testPort = 5060;
    char tmp[16];
    uint16_t mappedPort;

    int type = Stun::get_nat_type(**addr, stunPort, sock,
            localAddr, testPort, tmp, mappedPort);

    cerr << "The NAT type is "<< Stun::type_to_string(type)<< endl;
    cerr << "External mapping is: "<< tmp << ":" << mappedPort << endl;
    return 0;
}
