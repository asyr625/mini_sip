#ifndef TLS_SERVER_SOCKET_H
#define TLS_SERVER_SOCKET_H
#include "server_socket.h"
#include "cert.h"

class Tls_Server_Socket : public Server_Socket
{
public:
    virtual ~Tls_Server_Socket();

    static Tls_Server_Socket *create( SRef<Server_Socket *> sock,
                    SRef<Certificate *> cert, SRef<Certificate_Set *> cert_db = NULL );
protected:
    Tls_Server_Socket();
};

#endif // TLS_SERVER_SOCKET_H
