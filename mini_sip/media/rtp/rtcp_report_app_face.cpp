#include "rtcp_report_app_face.h"
#include "mini_defines.h"
#include <iostream>
using namespace std;
Rtcp_Report_App_Face::Rtcp_Report_App_Face(uint32_t ssrc, int fcount, int fcountSinceLast )
    : Rtcp_Report(204), face_count_now(fcount), face_count_max_since_last_report(fcountSinceLast)
{
    rc_sc = 0;
    sender_ssrc = ssrc;
}

Rtcp_Report_App_Face::Rtcp_Report_App_Face(void * buildfrom, int max_length)
    : Rtcp_Report(204)
{
    if (max_length<4)
    {
        cerr <<"Too short RTCP APP report (in RtcpReportAPP constructor) (size="<<max_length<<")"<< endl;
        exit(1);
    }
    parse_header(buildfrom,max_length);

    int *iptr = &((int *)buildfrom)[1];
    sender_ssrc = U32_AT( iptr );

    int8_t *i8ptr = &((int8_t *)buildfrom)[12];
    face_count_now = i8ptr[0];
    face_count_max_since_last_report = i8ptr[1];
}

Rtcp_Report_App_Face::~Rtcp_Report_App_Face()
{
}

int Rtcp_Report_App_Face::get_length()
{
    return 4 + 12;
}

void Rtcp_Report_App_Face::write_data(char* buf)
{
    uint32_t* ub = (uint32_t*)buf;
    int8_t* i8 = (int8_t*)buf;
    uint16_t* u16b = (uint16_t*)buf;

    write_header(buf);

    buf[4]='F';
    buf[5]='A';
    buf[6]='C';
    buf[7]='E';

    i8[8] = face_count_now;
    i8[9] = face_count_max_since_last_report;
    i8[10]= 0;
    i8[11]= 0;
}

void Rtcp_Report_App_Face::debug_print()
{
    std::cerr<<"RTCP APP report type "<< rc_sc <<":"<< std::endl;
    std::cerr<< "\tsender_ssrc: "<< sender_ssrc<< std::endl;
    std::cerr<< "\tfacecountNow: "<< face_count_now << " facecountMaxSinceLastReport: "<< face_count_max_since_last_report << std::endl;
}

unsigned int Rtcp_Report_App_Face::get_subtype()
{
    return rc_sc;
}

unsigned int Rtcp_Report_App_Face::get_sender_ssrc()
{
    return sender_ssrc;
}

int8_t Rtcp_Report_App_Face::get_face_count_now()
{
    return face_count_now;
}

int8_t Rtcp_Report_App_Face::get_face_count_max_since_last_report()
{
    return face_count_max_since_last_report;
}
