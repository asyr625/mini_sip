#include "mini_defines.h"
#include "my_types.h"
#include "rtcp_report.h"

Rtcp_Report::Rtcp_Report(unsigned pt)
    : packet_type(pt)
{
    this->version = 2;
    this->padding = 0;
    this->length = 0;
    this->rc_sc = 0;
}


void Rtcp_Report::parse_header(void *build_from, int max_length)
{
    uint8_t * bytearray = (uint8_t *)build_from;

    this->version = bytearray[0] >> 6;
    this->padding = bytearray[0] >> 5 & 0x1;
    this->rc_sc = bytearray[0] & 0x1F;
    this->packet_type = bytearray[1];
    this->length = U16_AT(bytearray + 2);
}

int Rtcp_Report::write_header(char* buf)
{
    byte_t* b = (byte_t*)buf;
    uint16_t* s = (uint16_t*)buf;
    b[0] = version<<6 | padding<<5 | rc_sc;

    b[1] = packet_type;
    length = get_length()/4-1;
    s[1] = hton16( length );

    return 4;
}
