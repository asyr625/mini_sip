#ifndef RTCP_REPORT_APP_FACE_H
#define RTCP_REPORT_APP_FACE_H

#include "my_types.h"
#include "rtcp_report.h"

class Rtcp_Report_App_Face : public Rtcp_Report
{
public:
    Rtcp_Report_App_Face(uint32_t ssrc, int fcount, int fcountSinceLast );
    Rtcp_Report_App_Face(void * buildfrom, int max_length);
    virtual ~Rtcp_Report_App_Face();

    int get_length();
    void write_data(char* buf);

    virtual void debug_print();

    unsigned int get_subtype();
    unsigned int get_sender_ssrc();
    int8_t get_face_count_now();
    int8_t get_face_count_max_since_last_report();

private:
    uint32_t sender_ssrc;
    int8_t face_count_now;
    int8_t face_count_max_since_last_report;
};

#endif // RTCP_REPORT_APP_FACE_H
