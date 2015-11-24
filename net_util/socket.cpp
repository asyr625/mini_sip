#include "socket.h"
#include "network_exception.h"
#include "my_types.h"
#include "my_assert.h"
#if defined _MSC_VER || defined __MINGW32__	// was: if defined WIN32 && not defined __CYGWIN__
#include<winsock2.h>
# define USE_WIN32_API
#else
#include <unistd.h>
#include <sys/socket.h>
#endif
#include <errno.h>
#include <iostream>
using namespace std;

#ifdef USE_WIN32_API
typedef int socklen_t;
#endif
Socket::Socket()
{
}

Socket::~Socket()
{
    if( _fd != -1 )
    {
        this->close();
    }
}

int Socket::get_fd()
{
    return _fd;
}

int Socket::get_type()
{
    return _type;
}

int Socket::get_port()
{
    SRef<IPAddress *> addr = get_local_address();
    int32_t port2 = addr->get_port();
    return port2;
}

int Socket::get_address_family()
{
    struct sockaddr_storage sa;
    socklen_t salen = sizeof(sa);

    if( getsockname(_fd, (struct sockaddr*)&sa, &salen) < 0 )
    {
        return -1;
    }

    return sa.ss_family;
}

SRef<IPAddress *> Socket::get_local_address() const
{
    struct sockaddr_storage sa;
    socklen_t sz = sizeof(sa);
    if (getsockname(_fd, (struct sockaddr *)&sa, &sz))
    {
        throw Getsockname_Failed( errno );
    }

    SRef<IPAddress *> addr = IPAddress::create((struct sockaddr*)&sa, sz);
    return addr;
}

bool Socket::set_ipv6_dscp(const std::bitset<6> &dscp)
{
    cerr << "Socket::setIPv6DSCP() error, this socket does not support this functionality." << endl;
    return false;
}

void Socket::close( void )
{
#ifdef USE_WIN32_API
    closesocket(_fd);
#else
    ::close(_fd);
#endif
    _fd = -1;
}
