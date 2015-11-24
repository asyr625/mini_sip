#include "rtcp_report_fir.h"
#include "mini_defines.h"
#include <iostream>
using namespace std;

Rtcp_Report_Fir::Rtcp_Report_Fir(uint32_t ssrc)
    : Rtcp_Report(PACKET_TYPE_FIR), ssrc(ssrc)
{

}

Rtcp_Report_Fir::Rtcp_Report_Fir(void* build_from, int max_length)
    : Rtcp_Report(PACKET_TYPE_FIR)
{
    if (max_length<4)
    {
        cerr <<"Too short RTCP FIR report (in RtcpReportFIR constructor) (size="<<max_length<<")"<< endl;
        exit(1);
    }
    parse_header(build_from,max_length);

    uint32_t* lb = (uint32_t*)build_from;
    ssrc = U32_AT( &lb[1] );
}

Rtcp_Report_Fir::~Rtcp_Report_Fir()
{
}

int Rtcp_Report_Fir::get_length()
{
    return 4 + 4;
}

void Rtcp_Report_Fir::write_data(char* buf)
{
    int i = 0;
    i += write_header(buf);
    uint32_t* lb = (uint32_t*)buf;
    lb[1]=hton32(ssrc);
}

void Rtcp_Report_Fir::debug_print()
{
    std::cerr<< "RTCP FIR request, ssrc=" << ssrc << std::endl;
}

uint32_t Rtcp_Report_Fir::get_ssrc()
{
    return ssrc;
}
