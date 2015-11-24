#include "rtp_header.h"
#include "mini_defines.h"

#include <iostream>

Rtp_Header::Rtp_Header()
    : version(0),
      extension(0),
      csrc_count(0),
      marker(0),
      payload_type(0),
      sequence_number(0),
      timestamp(0),
      ssrc(0)
{
#ifdef TCP_FRIENDLY
    tcp_friendly_mode=false;
    sending_timestamp=0;
    rttestimate=0;
#endif
}

void Rtp_Header::set_version(int v)
{
    this->version = v;
}

void Rtp_Header::set_extension(int x)
{
    this->extension =  x;
}

int Rtp_Header::get_extension()
{
    return extension;
}

void Rtp_Header::set_csrc_count(int cc)
{
    this->csrc_count = cc;
}

void Rtp_Header::set_marker(int m)
{
    this->marker = m;
}

bool Rtp_Header::get_marker()
{
    return marker == 1;
}

void Rtp_Header::set_payload_type(int pt)
{
    this->payload_type=pt;
}

int Rtp_Header::get_payload_type()
{
    return payload_type;
}

void Rtp_Header::set_seq_no(uint16_t seq_no)
{
    this->sequence_number=seq_no;
}

uint16_t Rtp_Header::get_seq_no()
{
    return sequence_number;
}

void Rtp_Header::set_timestamp(uint32_t t)
{
    this->timestamp = t;
}

uint32_t Rtp_Header::get_timestamp()
{
    return timestamp;
}

void Rtp_Header::set_ssrc(uint32_t ssrc)
{
    this->ssrc = ssrc;
}

uint32_t Rtp_Header::get_ssrc()
{
    return ssrc;
}

void Rtp_Header::add_ssrc(int c)
{
    csrc.push_back(c);
}

void Rtp_Header::print_debug()
{
    std::cerr << "\tversion: "<< version<<"\n\textension: "<< extension <<"\n\tCSRC count: "
              << csrc_count << "\n\tmarker: "<< marker << "\n\tpayload type: "<<payload_type <<"\n\tsequence number: "
              <<sequence_number << "\n\ttimestamp: "<<timestamp <<"\n\tSSRC: "<< ssrc << "\n"<< std::endl;

    for (int i=0; i< csrc_count; i++)
        std::cerr << "\tCSRC "<<i+1 << ": "<<csrc[i]<< std::endl;
}

int Rtp_Header::size()
{
    int s = 12+4*(int)csrc.size();

#ifdef TCP_FRIENDLY
    s += 8; // 4(rtt)+4(sendts)
#endif
    return s;
}

char *Rtp_Header::get_bytes()
{
    uint8_t i;
    char *ret = new char[size()];

    ret[0] = ( ( version << 6 ) & 0xc0 ) |
            ( ( extension << 4 ) & 0x10 ) |
            ( ( csrc_count & 0x0F ) );

    ret[1] = ( ( marker << 7 ) & 0x80 ) |
            ( ( payload_type & 0x7F ) );

    ret[2] = ( sequence_number >> 8 ) & 0xFF;

    ret[3] = ( sequence_number ) & 0xFF;

    ((uint32_t *)ret)[1] = hton32( timestamp );
    ((uint32_t *)ret)[2] = hton32( ssrc );

    int i32=3; //index in the packet where we are adding headers. If
    //we are using tcp friendly mode we insert two extra
    //32 bit headers and the CSRCs will not be in their
    //usual place.
#ifdef TCP_FRIENDLY
    if (tcp_friendly_mode)
    {
        ((uint32_t *)ret)[i32++] = hton32(sending_timestamp);
        ((uint32_t *)ret)[i32++] = hton32(rttestimate);
    }
#endif

    for( i = 0; i < csrc.size(); i++ )
        ((uint32_t *)ret)[i32+i]=hton32(csrc[i]);

    return ret;
}
