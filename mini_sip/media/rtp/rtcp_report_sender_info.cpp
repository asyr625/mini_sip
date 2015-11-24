#include "rtcp_report_sender_info.h"
#include <stdlib.h>
Rtcp_Report_Sender_Info::Rtcp_Report_Sender_Info(void *buildfrom, int max_length)
{
    if (max_length<20)
    {
        cerr << "ERROR: to short SenderInfo report in RtcpReportSenderInfo"<< endl;
        exit(1);
    }

    unsigned int *iptr;

    iptr = (unsigned int *)buildfrom;

    this->ntp_msw = U32_AT( iptr );

    iptr++;
    this->ntp_lsw = U32_AT( iptr );

    iptr++;
    this->rtp_timestamp = U32_AT( iptr );

    iptr++;
    this->sender_packet_count = U32_AT( iptr );

    iptr++;
    this->sender_octet_count = U32_AT( iptr );
}

int Rtcp_Report_Sender_Info::get_length()
{
    return 20;
}

void Rtcp_Report_Sender_Info::write_data(char* buf)
{
    uint32_t* lp = (uint32_t*)buf;
    lp[0] = hton32(ntp_msw);
    lp[1] = hton32(ntp_lsw);
    lp[2] = hton32(rtp_timestamp);
    lp[3] = hton32(sender_packet_count);
    lp[4] = hton32(sender_octet_count);
}

void Rtcp_Report_Sender_Info::debug_print()
{
    std::cerr << " sender info:"<< std::endl;
    std::cerr.setf(std::ios::hex, std::ios::basefield);
    std::cerr << "\tntp_msw: 0x"<< ntp_msw<< std::endl;
    std::cerr << "\tntp_lsw: 0x"<< ntp_lsw<< std::endl;
    std::cerr << "\trtp_timestamp: 0x"<< rtp_timestamp<< std::endl;
    std::cerr.setf(std::ios::dec, std::ios::basefield);
    std::cerr << "\tsender_packet_count: "<< sender_packet_count<< std::endl;
    std::cerr << "\tsender_octet_count: "<< sender_octet_count<< std::endl;
}

void Rtcp_Report_Sender_Info::set_ntp_timestamp_msw(unsigned t)
{
    ntp_msw = t;
}

unsigned Rtcp_Report_Sender_Info::get_ntp_timestamp_msw()
{
    return ntp_msw;
}

void Rtcp_Report_Sender_Info::set_ntp_timestamp_lsw(unsigned t)
{
    ntp_lsw = t;
}

unsigned Rtcp_Report_Sender_Info::get_ntp_timestamp_lsw()
{
    return ntp_lsw;
}

void Rtcp_Report_Sender_Info::set_rtp_timestamp(unsigned t)
{
    rtp_timestamp = t;
}

unsigned Rtcp_Report_Sender_Info::get_rtp_timestamp()
{
    return rtp_timestamp;
}

void Rtcp_Report_Sender_Info::set_sender_packet_count(int i)
{
    sender_packet_count = i;
}

unsigned Rtcp_Report_Sender_Info::get_sender_packet_count()
{
    return sender_packet_count;
}

void Rtcp_Report_Sender_Info::set_sender_octet_count(int i)
{
    sender_octet_count = i;
}

unsigned Rtcp_Report_Sender_Info::get_sender_octet_count()
{
    return sender_octet_count;
}
