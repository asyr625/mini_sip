#include "stun_attribute.h"

#include<assert.h>
#include<iostream>
#include<stdlib.h>
#include<string.h>

using namespace std;

#ifdef WIN32
#include<winsock2.h>
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include<netinet/in.h>
#include<netdb.h>
#endif

#include "string_utils.h"

const int Stun_Attribute::MAPPED_ADDRESS=0x0001;
const int Stun_Attribute::RESPONSE_ADDRESS=0x0002;
const int Stun_Attribute::CHANGE_REQUEST=0x0003;
const int Stun_Attribute::SOURCE_ADDRESS=0x0004;
const int Stun_Attribute::CHANGED_ADDRESS=0x0005;
const int Stun_Attribute::USERNAME=0x0006;
const int Stun_Attribute::PASSWORD=0x0007;
const int Stun_Attribute::MESSAGE_INTEGRITY=0x0008;
const int Stun_Attribute::ERROR_CODE=0x0009;
const int Stun_Attribute::UNKNOWN_ATTRIBUTES=0x000a;
const int Stun_Attribute::REFLECTED_FROM=0x000b;


#define CHANGE_IP_MASK 0x04
#define CHANGE_PORT_MASK 0x02


Stun_Attribute::Stun_Attribute(int t)
    : type(t)
{
}

Stun_Attribute::~Stun_Attribute()
{
}

int Stun_Attribute::get_type()
{
    return type;
}

int Stun_Attribute::get_message_data_tlv(unsigned char *buf)
{
    uint16_t *ptr= (uint16_t *)buf;
    ptr[0]=htons(type);
    ptr[1]=htons(get_value_length());
    get_value(&buf[4]);
    return 2 + 2 + get_value_length();
}

Stun_Attribute *Stun_Attribute::parse_attribute(unsigned char *data, int maxLength /*maxLength*/, int &retParsedLength)
{
    //TODO: use maxLength
    uint16_t *shortptr = (uint16_t *)data;
    int att_type   = ntohs(shortptr[0]);
    int length = ntohs(shortptr[1]);
    Stun_Attribute *ret = NULL;
    switch( att_type )
    {
    case Stun_Attribute::MAPPED_ADDRESS:
        ret = new Stun_Attribute_Mapped_Address(length,&data[4]);
        break;
    case Stun_Attribute::SOURCE_ADDRESS:
        ret = new Stun_Attribute_Source_Address(length, &data[4]);
        break;
    case Stun_Attribute::RESPONSE_ADDRESS:
        ret = new Stun_Attribute_Response_Address(length,&data[4]);
        break;
    case Stun_Attribute::CHANGED_ADDRESS:
        ret = new Stun_Attribute_Changed_Address(length,&data[4]);
        break;
    case Stun_Attribute::CHANGE_REQUEST:
        ret = new Stun_Attribute_Change_Request(&data[4], length);
        break;
    case Stun_Attribute::USERNAME:
        ret = new Stun_Attribute_Username(length,&data[4]);
        break;
    case Stun_Attribute::PASSWORD:
        ret = new Stun_Attribute_Password(length,&data[4]);
        break;
    case Stun_Attribute::ERROR_CODE:
        ret = new Stun_Attribute_Error_Code(length,&data[4]);
        break;
    default:
        if( att_type <= 0x7fff )
        { // attribute is mandatory to understand
            std::cerr << "STUN: UNKNOWN ATTRIBUTE: "<< att_type << std::endl;
            assert(1==0 /*UNKNOWN ATTRIBUTE*/);
        }
        else
        {             // attribute is not mandatory to understand, ignore
            retParsedLength = 2+2+length; // TODO: check that there is length to parse
            return NULL;
        }
        break;
    }
    assert( ret != NULL );
    retParsedLength = 2+2+ret->get_value_length();
    //  cerr << retParsedLength<<endl;
    return ret;
}


Stun_Attribute_Address::Stun_Attribute_Address(int type, uint16_t port, char *addr)
    : Stun_Attribute(type)
{
    assert(addr != NULL);

    struct hostent *hp= gethostbyname(addr);

    if (!hp){
        cerr << "Could not resolve host "<< addr << endl;
        //TODO: throw exception
        exit(1);
    }
    this->address = ntohl( * ((uint32_t*)hp->h_addr_list[0]) );

    this->port = port;
}

Stun_Attribute_Address::Stun_Attribute_Address(int type, unsigned char *data, int length)
    : Stun_Attribute(type)
{
    assert(length == 8); //FIXME: throw exception
    family = data[1];
    uint16_t *shortPtr = (uint16_t*)data;
    port = ntohs(shortPtr[1]);
    uint32_t *ptr = (uint32_t*)data;
    address = ntohl(ptr[1]);
}

std::string Stun_Attribute_Address::get_desc()
{
    uint32_t nip = htonl(address);

    std::string ip = std::string("") + itoa((nip >> 24)&0xFF)+"." + itoa((nip >>16)&0xFF)+"." + itoa((nip>>8)&0xFF)+"." + itoa(nip&0xFF);

    return std::string("Type: ") + itoa(get_type()) + std::string("; Family: ") + itoa(family) + std::string("; port: ") + itoa(port)+"; address: " + ip;
}

uint32_t Stun_Attribute_Address::get_binary_ip()
{
    return address;
}

uint16_t Stun_Attribute_Address::get_port()
{
    return port;
}

int Stun_Attribute_Address::get_value(unsigned char *buf)
{
    buf[0]=0;
    buf[1]=family;
    uint16_t *shortPtr = (uint16_t*)buf;
    shortPtr[1] = htons(port);
    uint32_t *ptr = (uint32_t*)buf;
    ptr[1]=htonl(address);
    return get_value_length();
}

int Stun_Attribute_Address::get_value_length()
{
    return 8;
}




Stun_Attribute_Mapped_Address::Stun_Attribute_Mapped_Address(char *addr, uint16_t port)
    : Stun_Attribute_Address(Stun_Attribute::MAPPED_ADDRESS, port, addr)
{
}

Stun_Attribute_Mapped_Address::Stun_Attribute_Mapped_Address(int length, unsigned char *data)
    : Stun_Attribute_Address(Stun_Attribute::MAPPED_ADDRESS, data, length)
{
}

Stun_Attribute_Response_Address::Stun_Attribute_Response_Address(char *addr, uint16_t port)
    : Stun_Attribute_Address(Stun_Attribute::RESPONSE_ADDRESS, port, addr)
{
}

Stun_Attribute_Response_Address::Stun_Attribute_Response_Address(int length, unsigned char *data)
    : Stun_Attribute_Address(Stun_Attribute::RESPONSE_ADDRESS, data, length)
{
}

Stun_Attribute_Changed_Address::Stun_Attribute_Changed_Address(char *addr, uint16_t port)
    : Stun_Attribute_Address(Stun_Attribute::CHANGE_REQUEST, port, addr)
{
}

Stun_Attribute_Changed_Address::Stun_Attribute_Changed_Address(int length, unsigned char *data)
    : Stun_Attribute_Address(Stun_Attribute::CHANGE_REQUEST, data, length)
{

}

Stun_Attribute_Source_Address::Stun_Attribute_Source_Address(char *addr, uint16_t port)
    : Stun_Attribute_Address(Stun_Attribute::SOURCE_ADDRESS, port, addr)
{
}

Stun_Attribute_Source_Address::Stun_Attribute_Source_Address(int length, unsigned char *data)
    : Stun_Attribute_Address(Stun_Attribute::SOURCE_ADDRESS, data, length)
{

}
