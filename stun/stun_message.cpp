#include "stun_message.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#ifdef _MSC_VER
#	include<winsock2.h>
#	include<io.h>
#else
#	include<unistd.h>
#		include<netinet/in.h>
#	ifdef HAVE_NETINET_IN_H
#	endif
#endif

Message_Header::Message_Header(int type)
    : message_type(type), message_length(0)
{
    for (int i=0; i< 16; i++)
        transaction_id[i] = rand()%256;
}

void Message_Header::set_payload_length(int len)
{
    message_length = len;
}

int Message_Header::get_payload_length()
{
    return message_length;
}

void Message_Header::set_type(int type)
{
    message_type = type;
}

void Message_Header::set_transaction_id(unsigned char *id)
{
    for (int i=0; i< 16; i++)
        transaction_id[i] = id[i];
}

int Message_Header::get_data(unsigned char*buf)
{
    uint16_t *p = (uint16_t *)buf;

    p[0] = htons(message_type);
    p[1] = htons(message_length);

    for (int i=0; i<16; i++)
        buf[4+i]=transaction_id[i];

    return get_header_length();
}


const int Stun_Message::BINDING_REQUEST = 0x0001;
const int Stun_Message::BINDING_RESPONSE = 0x0101;
const int Stun_Message::BINDING_ERROR_RESPONSE = 0x0111;
const int Stun_Message::SHARED_SECRET_REQUEST = 0x0002;
const int Stun_Message::SHARES_SECRET_RESPONSE = 0x0102;
const int Stun_Message::SHARED_SECRET_ERROR_RESPONSE = 0x0112;

Stun_Message::Stun_Message(unsigned char*buf, int length)
    : header(0)
{
    uint16_t *sptr = (uint16_t*)buf;
    header.set_payload_length(ntohs(sptr[1]));
    header.set_transaction_id(&buf[4]);
    uint16_t type = ntohs(*sptr);
    header.set_type(type);

    parse_attributes(&buf[20], header.get_payload_length());
}

Stun_Message::Stun_Message(int type)
    : header(type)
{
}

Stun_Message::~Stun_Message()
{
}

bool Stun_Message::same_transaction_id(Stun_Message &msg)
{
    for (int i=0; i<16; i++)
        if (header.transaction_id[i]!=msg.header.transaction_id[i])
            return false;
    return true;
}

void Stun_Message::add_attribute(Stun_Attribute *a)
{
    attributes.push_back(a);
}

unsigned char* Stun_Message::get_message_data(int &retLength)
{
    int length = 0;
    length += header.get_header_length();

    std::list<Stun_Attribute*>::iterator i;
    std::list<Stun_Attribute*>::iterator i_end;
    for( i = attributes.begin(); i != i_end; i++)
        length += 2 + 2 + (*i)->get_value_length(); //T+L+V

    header.set_payload_length(length - header.get_header_length());

    unsigned char *rawData = new unsigned char[length];

    int index = header.get_data(rawData);

    for( i = attributes.begin(); i != attributes.end(); i++)
        index += (*i)->get_message_data_tlv(&rawData[index]);

    assert( index == length );
    retLength = length;
    return rawData;
}

void Stun_Message::send_message(int fd)
{
    int retLength;
    unsigned char *data = get_message_data(retLength);

#ifdef _MSC_VER
    ::_write(fd,data, (unsigned int)retLength);
#else
    write(fd,data, retLength);
#endif
    delete []data;
}

Stun_Attribute *Stun_Message::get_attribute(int type)
{
    std::list<Stun_Attribute*>::iterator iter;
    std::list<Stun_Attribute*>::iterator iter_end = attributes.end();
    for(iter = attributes.begin(); iter != iter_end; iter++)
        if ((*iter)->get_type() == type)
            return *iter;
    return NULL;
}

void Stun_Message::parse_attributes(unsigned char *data, int length)
{
    int nleft = length;
    int index=0;
    do{
        int attrLen=0;
        Stun_Attribute *attr = Stun_Attribute::parse_attribute(&data[index], nleft, attrLen);
//		cerr << "Parsed Attribute: "<<attr->get_desc()<<endl;
        if (attr != NULL) add_attribute( attr );

        nleft -= attrLen;
        index += attrLen;
//		cerr << "nleft="<<nleft << endl;
    }while( nleft > 0 );
    assert( nleft == 0 );
}
