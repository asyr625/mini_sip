#ifndef DTLS_SOCKET_H
#define DTLS_SOCKET_H

#include "cert.h"
#include "datagram_socket.h"

#define SSOCKET_TYPE_DTLS_UDP     0x21

class Dtls_Socket : public Datagram_Socket
{
public:
    virtual ~Dtls_Socket();

    static Dtls_Socket* create( SRef<Datagram_Socket *> sock,
                   SRef<Certificate *> cert = NULL,
                   SRef<Certificate_Set *> cert_db = NULL );
protected:
    Dtls_Socket();
};

#endif // DTLS_SOCKET_H
