#ifndef TLS_SRP_SOCKET_H
#define TLS_SRP_SOCKET_H

#include <gnutls/gnutls.h>
#include <gnutls/extra.h>
#include <string>

#include "stream_socket.h"
#include "ipaddress.h"


class Tls_Srp_Socket : public Stream_Socket
{
public:
    Tls_Srp_Socket(std::string addrs, int32_t port, std::string user, std::string pass);
    virtual ~Tls_Srp_Socket();
    virtual int32_t write(const void *msg, int length);
    virtual int32_t write(std::string msg);
    virtual int32_t read (void *buf, int length);
  private:
    void TlsSrpSocketSrp_init(std::string addrs, int32_t port, std::string user, std::string pass);
    gnutls_session_t session;
    gnutls_srp_client_credentials_t srp_cred;
};

#endif // TLS_SRP_SOCKET_H
