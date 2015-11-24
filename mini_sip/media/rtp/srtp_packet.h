#ifndef SRTP_PACKET_H
#define SRTP_PACKET_H

#include "udp_socket.h"
#include "ipaddress.h"
#include "crypto_context.h"
#include "rtp_packet.h"

Rtp_Packet * read_rtp_packet( SRef<Crypto_Context *>, UDP_Socket * socket );

class SRtp_Packet : public Rtp_Packet
{
public:
    SRtp_Packet();
    SRtp_Packet(Crypto_Context *scontext, Rtp_Packet *rtppacket);
    SRtp_Packet(Rtp_Header hdr,
               unsigned char *content,
               int content_length,
               unsigned char *tag,
               int tag_length_,
               unsigned char *mki,
               int mki_length_);
    SRtp_Packet(unsigned char *content_, int clen,
                int seq_no, unsigned timestamp, unsigned ssrc);
    SRtp_Packet(Rtp_Header hdr, unsigned char *content, int content_length);
    virtual ~SRtp_Packet();

    static SRtp_Packet *read_packet( UDP_Socket &udp_sock,  SRef<IPAddress *>&from, int timeout = -1);

    static SRtp_Packet *read_packet( byte_t *buf, unsigned buflen);

    void protect( SRef<Crypto_Context *> scontext );
    int unprotect( SRef<Crypto_Context *> scontext );

    unsigned char *get_tag()       { return tag; }
    unsigned int get_tag_length()  { return tag_length; }
    void set_tag(unsigned char *t) { tag = t; }

    virtual char* get_bytes();
    virtual int size();
private:
    bool encrypted;
    unsigned char *tag;
    unsigned int tag_length;
    unsigned char * mki;
    unsigned int mki_length;
};

#endif // SRTP_PACKET_H
