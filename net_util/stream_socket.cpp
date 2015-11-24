#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "stream_socket.h"

Stream_Socket::Stream_Socket()
{
}

Stream_Socket::~Stream_Socket()
{
}

bool Stream_Socket::matches_peer(const IPAddress& address, int port) const
{
    return **_peer_address == address && port == _peer_port;
}

bool Stream_Socket::matches_peer(const std::string &address, int port) const
{
    if (_remote_host_unresolved.size()>0)
    {
        return _remote_host_unresolved == address && port == _peer_port;
    }
    else
    {
        return **_peer_address == **(IPAddress::create(address)) && port == _peer_port;
    }
}

SRef<IPAddress *> Stream_Socket::get_peer_address()
{
    return _peer_address;
}

int Stream_Socket::get_peer_port() const
{
    return _peer_port;
}

int Stream_Socket::write(std::string data)
{
#ifdef _MSC_VER
    return ::_write(_fd, data.c_str(), (unsigned int)data.length());
#else
    return ::send(_fd, data.c_str(), data.length(), 0);
#endif
}

int Stream_Socket::write(const void *buf, int count)
{
    return ::write(_fd, buf, count );
}

int Stream_Socket::read(void *buf, int count)
{
    return ::recv(_fd, (void*)buf, count, 0);
}
