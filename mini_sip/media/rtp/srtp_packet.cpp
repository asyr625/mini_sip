#include "srtp_packet.h"
#include "mini_defines.h"

#include <string.h>

void SRtp_Packet::protect( SRef<Crypto_Context *> scontext )
{
    /* Encrypt the packet */
    uint64_t index = ((uint64_t)scontext->get_roc() << 16) | (uint64_t)(get_header().get_seq_no());
    scontext->rtp_encrypt( this, index );
    encrypted = true;

    /* Compute MAC */
    tag_length = scontext->get_tag_length();
    tag = new unsigned char[ tag_length ];

    scontext->rtp_authenticate( this, scontext->get_roc(), tag );
    /* Update the ROC if necessary */
    if( get_header().get_seq_no() == 0xFFFF )
        scontext->set_roc( scontext->get_roc() + 1 );
}

int SRtp_Packet::unprotect( SRef<Crypto_Context *> scontext )
{
    if( !scontext ){
        // It's probably an RtpPacket
        return 0;
    }

    tag_length = scontext->get_tag_length();
    mki_length = scontext->get_mki_length();

    content_length -= (tag_length + mki_length);

    if (content_length < 0) {
#ifdef DEBUG_OUTPUT
        std::cerr << "unprotect failed: illegal packet length" << std::endl;
#endif
        return 1;
    }
    tag = content + content_length;
    mki = content + content_length + tag_length;

    // Content_length 0 may happen in case of extension header only and
    // possible checksum adjustment.
    if (content_length == 0)
        content = NULL;

    /* Guess the index */
    uint64_t guessed_index =
            scontext->guess_index( get_header().get_seq_no() );

    /* Replay control */
    if( !scontext->check_replay( get_header().get_seq_no() ) ){
#ifdef DEBUG_OUTPUT
        std::cerr << "replay check failed" << std::endl;
#endif
        tag = NULL;
        mki = NULL;
        return 1;
    }

    unsigned char * mac = new unsigned char[tag_length];
    scontext->rtp_authenticate( this, (uint32_t)( guessed_index >> 16 ), mac );
    for( unsigned i = 0; i < tag_length; i++ )
    {
        if( tag[i] != mac[i] )
        {
#ifdef DEBUG_OUTPUT
            std::cerr << "authentication failed in stream: " << get_header().getSSRC() << std::endl;
#endif
            tag = NULL;
            mki = NULL;
            return 1;
        }
    }
    delete [] mac;

    /* Decrypt the content */
    scontext->rtp_encrypt( this, guessed_index );
    encrypted = false;

    /* Update the Crypto-context */
    scontext->update( get_header().get_seq_no() );

    /* don't delete the tag and mki */
    tag = NULL;
    mki = NULL;

    return 0;
}


SRtp_Packet::SRtp_Packet()
    : tag(NULL),
      tag_length(0),
      mki(NULL),
      mki_length(0)
{
    content_length = 0;
    content = NULL;
}

SRtp_Packet::SRtp_Packet(Crypto_Context *scontext, Rtp_Packet *rtppacket)
{

}

SRtp_Packet::SRtp_Packet(Rtp_Header hdr, unsigned char *content, int content_length,
                         unsigned char *tag, int tag_length_, unsigned char *mki, int mki_length_)
    : Rtp_Packet(hdr, content, content_length ), encrypted(true), tag_length(tag_length_), mki_length(mki_length_)
{
    if(tag_length)
    {
        this->tag = new unsigned char[tag_length];
        memcpy( this->tag, tag, tag_length );
    }
    else
        this->tag = NULL;

    if(mki_length)
    {
        this->mki = new unsigned char[mki_length];
        memcpy( this->mki, mki, mki_length );
    }
    else
        this->mki = NULL;
}

SRtp_Packet::SRtp_Packet( unsigned char *content_, int clen,
                          int seq_no, unsigned timestamp, unsigned ssrc)
    : Rtp_Packet( content_, clen, seq_no, timestamp, ssrc ), tag(NULL), tag_length(0), mki(NULL), mki_length(0)
{

}

SRtp_Packet::SRtp_Packet(Rtp_Header hdr, unsigned char *content, int content_length)
    : Rtp_Packet(hdr, content, content_length ), encrypted(false), tag_length(0), mki_length(0)
{
    this->tag = NULL;
    this->mki = NULL;
}

SRtp_Packet::~SRtp_Packet()
{
    if( mki )
        delete [] mki;
    if( tag )
        delete [] tag;
}

SRtp_Packet *SRtp_Packet::read_packet( UDP_Socket &srtp_socket,  SRef<IPAddress *>&from, int timeout)
{
    int32_t port;
    int i = srtp_socket.recvfrom(srtp_socket.receive_buffer, UDP_SIZE, from, port);
    if( i < 0 )
    {
#ifdef DEBUG_OUTPUT
        my_error("recvfrom:");
#endif
        return NULL;
    }
    return read_packet(srtp_socket.receive_buffer, i);
}

SRtp_Packet *SRtp_Packet::read_packet( byte_t *buf, unsigned buflen)
{
    uint8_t j;
    uint8_t cc;

    if(buf == NULL)
        return NULL;

    cc = buf[0] & 0x0F;
    if( (int)buflen < 12 + cc * 4 )
    {
        /* too small to contain an RTP header with cc CCSRC */
        return NULL;
    }

    Rtp_Header hdr;
    hdr.set_version( ( buf[0] >> 6 ) & 0x03 );
    hdr.set_extension(  ( buf[0] >> 4 ) & 0x01 );
    hdr.set_csrc_count( cc );
    hdr.set_marker( ( buf[1] >> 7 ) & 0x01  );
    hdr.set_payload_type( buf[1] & 0x7F );

    hdr.set_seq_no( ( ((uint16_t)buf[2]) << 8 ) | buf[3] );
    hdr.set_timestamp( U32_AT( buf + 4 ) );
    hdr.set_ssrc( U32_AT( buf + 8 ) );

    for( j = 0 ; j < cc ; j++ )
        hdr.add_ssrc( U32_AT( buf + 12 + j*4 ) );

    int datalen = buflen - 12 - cc*4;

    unsigned char *data = (unsigned char *)&buf[ 12 + 4*cc ];

    //	std::cerr << "SRtpPacket::read_packet() read packet No " << hdr.get_seq_no() << " at " << mtime() << "ms" << std::endl;
    return new SRtp_Packet( hdr, data, datalen, NULL, 0, NULL, 0 );
}

char* SRtp_Packet::get_bytes()
{
    char* ret;
    ret = new char[ size() ];

    char* hdr = header.get_bytes();
    memcpy( ret, hdr, header.size() );
    delete [] hdr;

    int hsize = header.size();
    if (extension_length > 0)
    {
        memcpy(&ret[hsize], extension_header, extension_length);
    }
    hsize += extension_length;
    memcpy( &ret[hsize], content, content_length );
    memcpy( &ret[hsize + content_length], tag, tag_length );
    memcpy( &ret[hsize + content_length + tag_length], mki, mki_length);

    if (zrtp_checksum)
    {
        uint16_t chkSum = compute_checksum((uint16_t*)ret, size()-20);
        memcpy(&ret[hsize + content_length + tag_length + mki_length], &chkSum, zrtp_checksum);
    }
    return ret;
}

int SRtp_Packet::size()
{
    return header.size() + content_length + tag_length + mki_length + extension_length + zrtp_checksum;
}
