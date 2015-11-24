#include "rtp_packet.h"
#include "mini_defines.h"
#include <string.h>

Rtp_Packet::Rtp_Packet()
    : zrtp_checksum(0),
      content_length(0),
      content(NULL),
      extension_length(0),
      extension_header(NULL)
{
}

Rtp_Packet::Rtp_Packet(unsigned char *content_, int cl, int seq_no, unsigned timestamp, unsigned ssrc)
    : content_length(cl)
{
    extension_length = 0;
    extension_header = NULL;
    zrtp_checksum = 0;

    header.set_version(2);
    header.set_seq_no(seq_no);
    header.set_timestamp(timestamp);
    header.ssrc = ssrc;

    if( content_length )
    {
        my_assert(content_length>0 && content_length<0xFFFF);
        this->content = new unsigned char[content_length];
        memcpy(this->content, content_, content_length);
    }
    else
        this->content = NULL;
}

Rtp_Packet::Rtp_Packet(const Rtp_Header &hdr, unsigned char *content_, int cl)
    : header(hdr)
{
    extension_length = 0;
    extension_header = NULL;
    zrtp_checksum = 0;

    /*
     * Check if packet contains an extension header. If yes
     * set pointer to extension header, compute length and
     * adjust content pointer and content_length
     */
    if (header.get_extension() && cl >= 4)
    {
        extension_length = 4;
        cl -= 4;	// minimum size of extension header

        short tmp = *((short *)(content_+2));
        tmp = ntoh16(tmp);
        tmp *= 4;		// ext. header length is in words (4 bytes)
        extension_length += tmp;
        cl -= tmp;

        if (cl >= 0)
        {
            extension_header = new unsigned char[extension_length];
            memcpy(this->extension_header, content_, extension_length);
        }
    }
    this->content_length = cl;

    if( content_length > 0 )
    {
        this->content = new unsigned char[content_length];
        memcpy(this->content, content_ + extension_length, content_length);
    }
    else
        this->content = NULL;

    header.set_version(2);
}

Rtp_Packet::~Rtp_Packet()
{
    if (content != NULL)
        delete [] content;
    if (extension_header != NULL)
        delete [] extension_header;
}

Rtp_Packet *Rtp_Packet::read_packet(UDP_Socket &rtp_socket, int timeout)
{
    int i;
    uint8_t j;
    uint8_t cc;
    //	memset( buf, '\0', 2048 );

    i = rtp_socket.recv(rtp_socket.receive_buffer, UDP_SIZE );

    if( i < 0 ){
#ifdef DEBUG_OUTPUT
        my_error("recvfrom:");
#endif
        return NULL;
    }

    if( i < 12 )
    {
        /* too small to contain an RTP header */
        return NULL;
    }

    cc = rtp_socket.receive_buffer[0] & 0x0F;
    if( i < 12 + cc * 4 )
    {
        /* too small to contain an RTP header with cc CSRC */
        return NULL;
    }

    Rtp_Header hdr;
    hdr.set_version( ( rtp_socket.receive_buffer[0] >> 6 ) & 0x03 );
    hdr.set_extension(  ( rtp_socket.receive_buffer[0] >> 4 ) & 0x01 );
    hdr.set_csrc_count( cc );
    hdr.set_marker( ( rtp_socket.receive_buffer[1] >> 7 ) & 0x01  );
    hdr.set_payload_type( rtp_socket.receive_buffer[1] & 0x7F );

    hdr.set_seq_no( ( ((uint16_t)rtp_socket.receive_buffer[2]) << 8 ) | rtp_socket.receive_buffer[3] );
    //cerr << "GOT SEQN" << hdr.getSeqNo() << endl;

    int tmp = *((int *)(rtp_socket.receive_buffer + 4));
    tmp = ntoh32(tmp);
    hdr.set_timestamp(tmp);

    tmp = *((int *)(rtp_socket.receive_buffer + 8));
    tmp = ntoh32(tmp);
    hdr.set_ssrc(tmp);

    for( j = 0 ; j < cc ; j++ )
    {
        tmp = *((int *)(rtp_socket.receive_buffer + 12 + j*4));
        tmp = ntoh32(tmp);
        hdr.set_ssrc(tmp);
    }
    int datalen = i - 12 - cc*4;

    Rtp_Packet * rtp = new Rtp_Packet(hdr, (unsigned char *)&rtp_socket.receive_buffer[12+4*cc], datalen);

    return rtp;
}


void Rtp_Packet::sendto(UDP_Socket &udp_sock, const IPAddress &to_addr, int port)
{
    char *bytes = get_bytes();
    if(udp_sock.sendto(to_addr, port, bytes, size()) < 0)
        std::cerr << "Rtp_Packet::sendto() to " << to_addr.get_string() << ", port " << port << " failed" << std::endl;
    delete [] bytes;
}

Rtp_Header &Rtp_Packet::get_header()
{
    return header;
}

void Rtp_Packet::set_ext_header(unsigned char* data, int length)
{
    if (data == NULL || length == 0)
        return;

    extension_header = new unsigned char[length];
    memcpy(extension_header, data, length);

    extension_length = length;
    header.set_extension(1);
}

void Rtp_Packet::print_debug()
{
    std::cerr << "_RTP_Header_"<< std::endl;
    header.print_debug();
    std::cerr <<"_Content_"<< std::endl;
    std::cerr <<"\tContent length: "<< content_length<< std::endl;
}


bool Rtp_Packet::check_zrtp_checksum(bool )
{
    if (content_length >= 2) {
        content_length -= 2;
    }
    // TODO: implement the real recompute and check of checksum
    return true;
}


#define CKSUM_CARRY(x) (x = (x >> 16) + (x & 0xffff), (~(x + (x >> 16)) & 0xffff))
uint16_t Rtp_Packet::compute_checksum(uint16_t* data, int length)
{
    uint32_t sum = 0;
    uint16_t ans = 0;

    while (length > 1)
    {
        sum += *data++;
        length -= 2;
    }
    if (length == 1)
    {
        *(uint8_t *)(&ans) = *(uint8_t*)data;
        sum += ans;
    }

    uint16_t ret = CKSUM_CARRY(sum);
    /*
    * Return the inverted 16-bit result.
    */
    return (ret);
}

char *Rtp_Packet::get_bytes()
{
    char *ret = new char[size()];

    char *hdr = header.get_bytes();
    int hdrSize = header.size();
    memcpy(ret, hdr, hdrSize);
    delete [] hdr;

    if (extension_length > 0)
        memcpy(&ret[hdrSize], extension_header, extension_length);

    hdrSize += extension_length;
    memcpy(&ret[hdrSize], content, content_length);

    if (zrtp_checksum)
    {
        uint16_t chkSum = compute_checksum((uint16_t*)ret, size()-20);
        memcpy(&ret[hdrSize + content_length], &chkSum, zrtp_checksum);
    }
    return ret;
}

int Rtp_Packet::size()
{
    return header.size() + content_length + extension_length + zrtp_checksum;
}
