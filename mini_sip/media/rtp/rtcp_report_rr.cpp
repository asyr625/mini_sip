#include "rtcp_report_rr.h"
#include "mini_defines.h"
#include <iostream>
using namespace std;

Rtcp_Report_RR::Rtcp_Report_RR(unsigned sender_ssrc)
    : Rtcp_Report(201),sender_ssrc(sender_ssrc)
{

}

Rtcp_Report_RR::Rtcp_Report_RR(void * build_from, int max_length)
    : Rtcp_Report(201)
{
    if (max_length < 4)
    {
        cerr <<"Too short RTCP RR report (in RtcpReportRR constructor) (size="<<max_length<<")"<< endl;
        exit(1);
    }
    parse_header(build_from,max_length);

    sender_ssrc = U32_AT( (uint8_t*)build_from + 1 );

    int i=8;
    while (i < max_length-1)
    {
        Rtcp_Report_Reception_Block block(& ((char*)build_from)[i], max_length-i);
        reception_blocks.push_back(block);
        i+=block.get_length();
    }
}

Rtcp_Report_RR::~Rtcp_Report_RR()
{
}

int Rtcp_Report_RR::get_length()
{
    int totsize=4;
    for (unsigned i=0; i< reception_blocks.size(); i++)
        totsize+=reception_blocks[i].get_length();
    return totsize;
}

void Rtcp_Report_RR::write_data(char* buf)
{
    my_assert(1 == 0);
}

void Rtcp_Report_RR::debug_print()
{
    std::cerr<<"RTCP RR report:"<< std::endl;
    std::cerr.setf( std::ios::dec, std::ios::basefield );
    std::cerr << "Sender ssrc="<<sender_ssrc << std::endl;
    std::cerr.setf( ios::hex, ios::basefield );
    for (unsigned i=0; i<reception_blocks.size(); i++)
        reception_blocks[i].debug_print();
}

int Rtcp_Report_RR::get_nreport_blocks()
{
    return (int)reception_blocks.size();
}

Rtcp_Report_Reception_Block &Rtcp_Report_RR::get_reception_block(int i)
{
    return reception_blocks[i];
}

void Rtcp_Report_RR::add_reception_block(Rtcp_Report_Reception_Block block)
{
    reception_blocks.push_back(block);
}
