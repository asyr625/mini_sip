#include "rtcp_report_xr.h"

Rtcp_Report_XR::Rtcp_Report_XR(void *build_from, int max_length)
    : Rtcp_Report(0)
{
    if (max_length<4)
    {
        std::cerr <<"Too short RTCP SR report (in RtcpReportSR constructor) (size="<<max_length<<")"<< std::endl;
        exit(1);
    }
    parse_header(build_from, max_length);

    int buflen = length*4;

    int *iptr = &(((int *)build_from)[1]);
    ssrc_or_csrc = U32_AT( iptr );

    int i=8;

    while (i<buflen-4)
    {
        XR_Report_Block *block = XR_Report_Block::build_from(& ((char*)build_from)[i], buflen-i);
        xr_blocks.push_back(block);
        i+=block->size();
    }
}

void Rtcp_Report_XR::debug_print()
{
    std::cerr << "RtcpReportXR:"<< std::endl;
    std::cerr.setf( std::ios::hex, std::ios::basefield );
    std::cerr << "ssrc_or_csrc: 0x"<< ssrc_or_csrc << std::endl;
    std::cerr.setf( std::ios::dec, std::ios::basefield );
    for (unsigned i=0; i<xr_blocks.size(); i++)
        xr_blocks[i]->debug_print();
}

int Rtcp_Report_XR::get_length()
{
    int totsize = 8;
    for (unsigned i=0; i<xr_blocks.size(); i++)
        totsize += xr_blocks[i]->size();

    return totsize;
}

void Rtcp_Report_XR::write_data(char* buf)
{
    my_assert(1==0);
}
