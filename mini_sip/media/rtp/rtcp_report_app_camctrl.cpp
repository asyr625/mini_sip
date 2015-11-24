#include "rtcp_report_app_camctrl.h"
#include "mini_defines.h"

Rtcp_Report_App_Camctrl::Rtcp_Report_App_Camctrl(unsigned subtype, uint32_t ssrc, int8_t horizontalSpeed,
                                                 int8_t verticalSpeed, int8_t zoomSpeed, uint8_t durationHundredsS)
    : Rtcp_Report(204),
      sender_ssrc(ssrc),
      horizontal_speed(horizontalSpeed),
      vertical_speed(verticalSpeed),
      zoom_speed(zoomSpeed),
      duration_hundredsS(durationHundredsS)
{
    rc_sc = subtype;
}

Rtcp_Report_App_Camctrl::Rtcp_Report_App_Camctrl(void * buildfrom, int max_length)
    : Rtcp_Report(204)
{
    if (max_length < 4)
    {
        std::cerr <<"Too short RTCP APP report (in RtcpReportAPP constructor) (size="<<max_length<<")"<< std::endl;
        //exit(1);
    }
    parse_header(buildfrom,max_length);

    uint32_t* lb = (uint32_t*)buildfrom;
    uint8_t* ub = (uint8_t*)buildfrom;
    int8_t* sb = (int8_t*)buildfrom;

    sender_ssrc = U32_AT( &lb[1] );

    horizontal_speed = sb[12];
    vertical_speed = sb[13];
    zoom_speed = sb[14];
    duration_hundredsS = ub[15];
}

Rtcp_Report_App_Camctrl::~Rtcp_Report_App_Camctrl()
{
}

int Rtcp_Report_App_Camctrl::get_length()
{
    return 4 + 12;
}

void Rtcp_Report_App_Camctrl::write_data(char* buf)
{
    uint32_t* lb = (uint32_t*)buf;
    uint8_t* ub = (uint8_t*)buf;
    int8_t* sb = (int8_t*)buf;

    write_header(buf);

    lb[1]=hton32(sender_ssrc);

    buf[8]='C';
    buf[9]='C';
    buf[10]='T';
    buf[11]='L';

    sb[12] = horizontal_speed;
    sb[13] = vertical_speed;
    sb[14] = zoom_speed;
    ub[15] = duration_hundredsS;
}

void Rtcp_Report_App_Camctrl::debug_print()
{
    std::cerr<<"RTCP APP report type "<< rc_sc <<":"<< std::endl;
    std::cerr<< "\thspeed: "<< (int)horizontal_speed << std::endl;
    std::cerr<< "\tvspeed: "<< (int)vertical_speed << std::endl;
    std::cerr<< "\tzspeed: "<< (int)zoom_speed<< std::endl;
    std::cerr<< "\tduration: "<< (int)duration_hundredsS<< std::endl;
}

unsigned int Rtcp_Report_App_Camctrl::get_subtype()
{
    return rc_sc;
}

unsigned int Rtcp_Report_App_Camctrl::get_senderssrc()
{
    return sender_ssrc;
}
