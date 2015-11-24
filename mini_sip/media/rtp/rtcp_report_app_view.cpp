#include "rtcp_report_app_view.h"
#include "mini_defines.h"

Rtcp_Report_App_View::Rtcp_Report_App_View(unsigned subtype, uint32_t ssrc, uint16_t width, uint16_t height)
    : Rtcp_Report(204)
{
    rc_sc = subtype;
    sender_ssrc = ssrc;
    sender_width = width;
    sender_height = height;
}

Rtcp_Report_App_View::Rtcp_Report_App_View(void * buildfrom, int max_length)
    : Rtcp_Report(204)
{
    if (max_length<4)
    {
        std::cerr <<"Too short RTCP APP report (in RtcpReportAPP constructor) (size="<<max_length<<")"<< std::endl;
        exit(1);
    }
    parse_header(buildfrom,max_length);

    int *iptr = &((int *)buildfrom)[1];
    //sender_ssrc = ntohl(*iptr);
    sender_ssrc = U32_AT( iptr );

    short *sptr = &((short *)buildfrom)[6];
    sender_width = U16_AT( sptr );

    sptr = &((short *)buildfrom)[7];
    sender_height = U16_AT( sptr );
}

Rtcp_Report_App_View::~Rtcp_Report_App_View()
{
}

int Rtcp_Report_App_View::get_length()
{
    return 4 + 12;
}

void Rtcp_Report_App_View::write_data(char* buf)
{
    uint32_t* ub = (uint32_t*)buf;
    uint16_t* u16b = (uint16_t*)buf;

    write_header(buf);

    ub[1] = hton32(sender_ssrc);

    buf[8]='V';
    buf[9]='I';
    buf[10]='E';
    buf[11]='W';

    u16b[6]=hton16(sender_width);
    u16b[7]=hton16(sender_height);
}

void Rtcp_Report_App_View::debug_print()
{
    std::cerr<< "RTCP APP report type "<< rc_sc <<":"<< std::endl;
    std::cerr<< "\tsender_ssrc: "<< sender_ssrc<< std::endl;
    std::cerr<< "\tsender WxH: "<< sender_width<< "x"<< sender_height<< std::endl;
}

unsigned int Rtcp_Report_App_View::get_subtype()
{
    return rc_sc;
}

unsigned int Rtcp_Report_App_View::get_sender_ssrc()
{
    return sender_ssrc;
}

int Rtcp_Report_App_View::get_sender_width()
{
    return sender_width;
}

int Rtcp_Report_App_View::get_sender_height()
{
    return sender_height;
}
