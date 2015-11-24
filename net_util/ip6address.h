#ifndef IP6ADDRESS_H
#define IP6ADDRESS_H

#include<iostream>

#include "ipaddress.h"

struct sockaddr_in6;

class IP6Address : public IPAddress
{
public:
    IP6Address(std::string addr);
    IP6Address(const IP6Address&other);
    IP6Address(struct sockaddr_in6 *addr);
    ~IP6Address();

    virtual int get_port() const;
    virtual std::string get_string() const;
    virtual void connect(Socket &socket, int port) const;
    friend std::ostream& operator<<(std::ostream&, const IP6Address &a);

    virtual struct sockaddr *get_sockaddr_ptr(int port=0) const;
    virtual int get_sockaddr_length() const;

    virtual bool operator ==(const IP6Address &i6) const;
    virtual bool operator ==(const IPAddress &i) const;

    virtual SRef<IPAddress *> clone() const;

private:
    std::string _ipaddr;
    unsigned short _num_ip[8];
    struct sockaddr_in6 * _sock_address;
};

#endif // IP6ADDRESS_H
