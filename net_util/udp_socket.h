#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H

#include <bitset>
#include "datagram_socket.h"

class UDP_Socket : public Datagram_Socket
{
public:
    UDP_Socket( int port=0, bool use_ipv6=false );
//		UDP_Socket( int port );

    virtual ~UDP_Socket();

    virtual std::string get_mem_object_type() const { return "UDP_Socket"; }

    int sendto(const IPAddress &to_addr, int port, const void *msg, int len);

    int recvfrom(void *buf, const int &len, SRef<IPAddress *> &from, int &port);

    int recv(void *buf, const int &len);

    bool set_low_delay();

    /* Allows to set DiffServ Code Point,
     * the part of the Traffic Class header field of the IP packet.
     */
    virtual bool set_ipv6_dscp(const std::bitset<6> &dscp);

private:
    bool init_udp_socket( bool use_ipv6, int port );
    bool use_ipv6;
};

#endif // UDP_SOCKET_H
