#ifndef DATAGRAM_SOCKET_H
#define DATAGRAM_SOCKET_H

#include "socket.h"

#define UDP_SIZE 65535

class Datagram_Socket : public Socket
{
public:
    virtual ~Datagram_Socket();

    virtual std::string get_mem_object_type() const = 0;

    virtual int sendto(const IPAddress &to_addr, int port, const void *msg, int len) = 0;

    virtual int recvfrom(void *buf, const int &len, SRef<IPAddress *>& from, int &port) = 0;

    virtual int recv(void *buf, const int &len) = 0;

    virtual bool set_low_delay() = 0;

    unsigned char receive_buffer[UDP_SIZE];
};

#endif // DATAGRAM_SOCKET_H
