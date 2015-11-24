#include "ip4address.h"
#include "net_config.h"
#include "my_defines.h"

#include "socket.h"
#include "network_exception.h"
#include "my_error.h"
#include "string_utils.h"

#include <iostream>
#include <cstring>
#include <exception>
#include <typeinfo>

#ifdef HAVE_NETDB_H
#include<netdb.h>
#endif
#ifdef HAVE_NETINET_IN_H
#include<netinet/in.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
#include<sys/socket.h>
#endif
#ifdef HAVE_ARPA_INET_H
#include<arpa/inet.h>
#endif

#ifndef HAVE_INET_ATON
#include<inet_aton.h>
#endif

using namespace std;

static std::string toStr(uint32_t numIp)
{
    unsigned char ip[4];
    ip[0] = (unsigned char)(numIp >> 24);
    ip[1] = (unsigned char)(numIp >> 16);
    ip[2] = (unsigned char)(numIp >>  8);
    ip[3] = (unsigned char)(numIp);
    std::string ret="(";
    for (int32_t i=0; i<4; i++)
    {
        if (i>0)
            ret=ret+".";
        ret = ret + itoa( (unsigned)ip[i] );
    }
    ret = ret+")";
    return ret;
}

std::ostream& operator<<(std::ostream& out, IP4Address &a)
{
    return out << toStr(a._num_ip);
}

Dbg& operator<<(Dbg&out, IP4Address &a)
{
    return out << toStr(a._num_ip);
}

IP4Address::IP4Address(std::string addr)
{
    _sock_address = new sockaddr_in;
    type = IP_ADDRESS_TYPE_V4;
    _ipaddr = addr;

    set_address_family(AF_INET);
    set_protocol_family(PF_INET);
    struct in_addr ip_data;
    if (inet_aton(addr.c_str(),&ip_data))
    {
        _num_ip = ntoh32(ip_data.s_addr);
    }else{

        //unsigned char *ip;

#ifndef WIN32
        struct hostent *hp= gethostbyname2(_ipaddr.c_str(), AF_INET);
#else
        struct hostent *hp= gethostbyname(_ipaddr.c_str());
//		struct hostent *hp= gethostbyaddr(_ipaddr.c_str(), 4, AF_INET);

        if (WSAGetLastError() != 0) {
            if (WSAGetLastError() == 11001)
                throw Host_Not_Found( addr );
        }
#endif
        if (!hp)
        {
            throw Host_Not_Found( addr );
        }

        _num_ip = ntoh32(*((uint32_t*)(hp->h_addr)));

        my_assert(hp->h_length == 4);
        #ifdef DEBUG_OUTPUT
        my_dbg("net") << "IP4Address(string): " << *this << endl;
        #endif
    }

    memset(_sock_address, '\0', sizeof(struct sockaddr_in));
    _sock_address->sin_family=AF_INET;
    _sock_address->sin_addr.s_addr = hton32(_num_ip);
    _sock_address->sin_port=0;
}

IP4Address::IP4Address(struct sockaddr_in *sin)
{
    _sock_address = new sockaddr_in;
    type = IP_ADDRESS_TYPE_V4;
    set_address_family(AF_INET);
    set_protocol_family(PF_INET);

    memcpy(_sock_address, sin, sizeof(struct sockaddr_in));
    _num_ip = ntoh32(sin->sin_addr.s_addr);
    _ipaddr = string(inet_ntoa(in_addr((sin->sin_addr))));
}

IP4Address::IP4Address(const IP4Address&other)
{
    type = IP_ADDRESS_TYPE_V4;
    set_address_family(AF_INET);
    set_protocol_family(PF_INET);
    _ipaddr = other._ipaddr;
    _num_ip = other._num_ip;
    _sock_address = new sockaddr_in;
    memcpy(_sock_address, other._sock_address, sizeof(struct sockaddr_in));
}

IP4Address::~IP4Address()
{
    if(NULL != _sock_address)
        delete _sock_address;
}

unsigned int IP4Address::get_binary_ip()
{
    return _num_ip;
}

int IP4Address::get_port() const
{
    return ntoh16(_sock_address->sin_port);
}

std::string IP4Address::get_string() const
{
    return _ipaddr;
}

void IP4Address::connect(Socket &socket, int port) const
{
    unsigned char *ip;
    unsigned long int ip_data;
    if (inet_aton(_ipaddr.c_str(),(struct in_addr *)((void*)&ip_data) ))
    {
        ip = (unsigned char *)&ip_data;
    }else{

#ifndef WIN32
        struct hostent *hp= gethostbyname2(_ipaddr.c_str(), AF_INET);
#else
        struct hostent *hp= gethostbyname(_ipaddr.c_str());
#endif
        if (!hp) //throw host not found exception here
        {
            throw Host_Not_Found( _ipaddr );
        }
        ip = (unsigned char *)hp->h_addr;
        my_assert(hp->h_length == 4);
    }

    struct sockaddr_in sin;
    memset(&sin, '\0', sizeof(sin));
    sin.sin_family = AF_INET;
    memcpy(&sin.sin_addr, ip, sizeof(ip_data));
    sin.sin_port = htons( (unsigned short)port );

    int error = ::connect(socket.get_fd(), (struct sockaddr *)&sin, sizeof(sin));
    if (error < 0)
    {
        int failedErrno = errno;
        my_error("connect");
        socket.close();
        throw Connect_Failed( failedErrno );
    }
}


struct sockaddr * IP4Address::get_sockaddr_ptr(int port) const
{
    _sock_address->sin_port = hton16(port); //BUG: we should not change this in a const object
    return (sockaddr *)_sock_address;
}
int IP4Address::get_sockaddr_length() const
{
    return sizeof(struct sockaddr_in);
}

bool IP4Address::operator ==(const IP4Address &i4) const
{
    return this->_num_ip == i4._num_ip;
}
bool IP4Address::operator ==(const IPAddress &i) const
{
    try{
        const IP4Address &i4 = dynamic_cast<const IP4Address&>(i);
        return (*this == i4);
    }
    catch(std::bad_cast &){
        // Comparing IPv6 and IPv4 addresses
        return false;
    }
}

SRef<IPAddress *> IP4Address::clone() const
{
    return new IP4Address(*this);
}
