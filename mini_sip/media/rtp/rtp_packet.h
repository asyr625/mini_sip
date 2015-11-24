#ifndef RTP_PACKET_H
#define RTP_PACKET_H
#include <iostream>
#include <stdio.h>

#include "sobject.h"
#include "udp_socket.h"
#include "ipaddress.h"

#include "rtp_header.h"

class Rtp_Packet : public SObject
{
public:
    friend class Rtp_Stream;

    Rtp_Packet();
    Rtp_Packet(unsigned char *content_, int cl, int seq_no, unsigned timestamp, unsigned ssrc);
    Rtp_Packet(const Rtp_Header &hdr, unsigned char *content_, int cl);
    virtual ~Rtp_Packet();

    static Rtp_Packet *read_packet(UDP_Socket &rtp_socket, int timeout=-1);

    Rtp_Header &get_header();

    unsigned char *get_content()  {return (content_length > 0) ? content : NULL;}
    int get_content_length() const {return content_length;}

    int get_extension_length() const { return extension_length; }
    unsigned char* get_extension_header() { return extension_header; }

    void set_ext_header(unsigned char* data, int length);

    void print_debug();

    void enable_zrtp_checksum() { zrtp_checksum = 2;}

    bool check_zrtp_checksum(bool);

    uint16_t compute_checksum(uint16_t* data, int length);

    virtual char *get_bytes();
    virtual int size();

protected:
    void sendto(UDP_Socket &udp_sock, const IPAddress &to_addr, int port);

    int zrtp_checksum;
    Rtp_Header header;
    int content_length;
    unsigned char *content;
    int extension_length;
    unsigned char* extension_header;
};

#endif // RTP_PACKET_H
