#include "inet_aton.h"
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
// borrowed from tcpdump
int
inet_aton(const char *cp, struct in_addr *addr)
{
    addr->s_addr = inet_addr(cp);
    return (addr->s_addr == INADDR_NONE) ? 0 : 1;
}
