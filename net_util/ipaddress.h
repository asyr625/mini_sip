#ifndef IPADDRESS_H
#define IPADDRESS_H

#define IP_ADDRESS_TYPE_V4      0
#define IP_ADDRESS_TYPE_V6      1

#include "sobject.h"
//#include "socket.h"

#ifdef WIN32
    #include <winsock2.h>
    #ifdef HAVE_WS2TCPIP_H
        # include<ws2tcpip.h>
    #endif
#elif defined HAVE_NETINET_IN_H
    #include<sys/types.h>
    #include<netinet/in.h>
    #include<sys/socket.h>
#endif

class Socket;

class IPAddress : public SObject
{
public:

    virtual ~IPAddress();

    int get_type() const { return type; }

    int get_address_family() const;
    int get_protocol_family() const;
    virtual int get_port() const=0;

    virtual std::string get_string() const=0;

    virtual void connect(Socket &s, int port) const=0;

    virtual struct sockaddr* get_sockaddr_ptr(int port=0) const=0;
    virtual int get_sockaddr_length() const=0;

    virtual bool operator ==(const IPAddress &i) const =0;

    virtual SRef<IPAddress *> clone() const =0;

    static SRef<IPAddress *> create(struct sockaddr * addr, int addr_len);

    static SRef<IPAddress *> create(const std::string &addr);

    static SRef<IPAddress *> create(const std::string &addr, bool use_ipv6);

    static bool is_numeric(const std::string &addr);

protected:
    void set_address_family(int af);
    void set_protocol_family(int pf);
    int type;

private:
    int _protocol_family;
    int _address_family;
};

#endif // IPADDRESS_H
