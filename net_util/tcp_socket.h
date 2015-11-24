#ifndef TCP_SOCKET_H
#define TCP_SOCKET_H

#include "stream_socket.h"

class TCP_Socket : public Stream_Socket
{
public:
    TCP_Socket(int fd, sockaddr * addr, int addr_len);

    TCP_Socket(std::string addr,int port=0);
    TCP_Socket(const IPAddress &ipaddress, int port=0);

    TCP_Socket(TCP_Socket &sock);
    virtual ~TCP_Socket();

    virtual std::string get_mem_object_type() const {return "TCPSocket";}

//		void flush();

    void use_nodelay(bool noDelay);

    friend std::ostream& operator<<(std::ostream&, TCP_Socket&);

    /* Allows to set DiffServ Code Point,
     * the part of the Traffic Class header field of the IP packet.
     */
    virtual bool set_ipv6_dscp(const std::bitset<6> &dscp);

private:
    void init_tcp_socket(const IPAddress &ipaddress, int port);
};

TCP_Socket& operator<<(TCP_Socket& sock, std::string str);

#endif // TCP_SOCKET_H
