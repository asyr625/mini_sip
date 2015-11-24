#ifndef TLS_SOCKET_H
#define TLS_SOCKET_H

#include "stream_socket.h"
#include "cert.h"

class Tls_Socket : public Stream_Socket
{
public:
    virtual ~Tls_Socket();

    static Tls_Socket* connect( SRef<Stream_Socket*> ssock,
                   SRef<Certificate *> cert = NULL,
                   SRef<Certificate_Set *> cert_db = NULL,
                   std::string serverName="" );

protected:
    Tls_Socket();
};

#endif // TLS_SOCKET_H
