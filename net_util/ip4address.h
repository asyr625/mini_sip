#ifndef IP4ADDRESS_H
#define IP4ADDRESS_H

#include "dbg.h"
#include "ipaddress.h"

struct sockaddr_in;

class IP4Address : public IPAddress
{
public:
    IP4Address(std::string addr);
    IP4Address(struct sockaddr_in *sin);
    IP4Address(const IP4Address&other);
    ~IP4Address();

    unsigned int get_binary_ip();

    virtual int get_port() const;
    virtual std::string get_string() const;
    virtual void connect(Socket &socket, int port) const;
    friend std::ostream& operator<<(std::ostream&, IP4Address &a);
    friend Dbg& operator<<(Dbg&, IP4Address &a);

    virtual struct sockaddr * get_sockaddr_ptr(int port=0) const;
    virtual int get_sockaddr_length() const;

    virtual bool operator ==(const IP4Address &i4) const;
    virtual bool operator ==(const IPAddress &i) const;

    virtual SRef<IPAddress *> clone() const;

private:
    std::string _ipaddr;
    struct sockaddr_in * _sock_address;
    unsigned int _num_ip;
};

#endif // IP4ADDRESS_H
