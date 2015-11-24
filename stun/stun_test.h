#ifndef STUN_TEST_H
#define STUN_TEST_H

#include "stun_message.h"

#include "udp_socket.h"
#include "ipaddress.h"

class Stun_Test
{
public:
    static Stun_Message *test(IPAddress *addr, uint16_t port, UDP_Socket &sock, bool changeIP, bool changePort);
};

#endif // STUN_TEST_H
