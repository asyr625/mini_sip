#ifndef STREAM_SOCKET_H
#define STREAM_SOCKET_H

#include "socket.h"

class Stream_Socket : public Socket
{
public:
    Stream_Socket();
    virtual ~Stream_Socket();
    virtual int write(std::string data);
    virtual int write(const void *buf, int count);
    virtual int read(void *buf, int count);

    // Buffer of the received data;
    std::string _received;

    bool matches_peer(const IPAddress& address, int port) const;
    bool matches_peer(const std::string &address, int port) const;

    SRef<IPAddress *> get_peer_address();
    int get_peer_port() const;

protected:

    SRef<IPAddress *> _peer_address;
    std::string _remote_host_unresolved;
    int _peer_port;
};

#endif // STREAM_SOCKET_H
