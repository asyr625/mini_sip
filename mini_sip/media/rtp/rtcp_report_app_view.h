#ifndef RTCP_REPORT_APP_VIEW_H
#define RTCP_REPORT_APP_VIEW_H
#include "my_types.h"
#include "rtcp_report.h"

class Rtcp_Report_App_View : public Rtcp_Report
{
public:
    Rtcp_Report_App_View(unsigned subtype, uint32_t ssrc, uint16_t width, uint16_t height);
    Rtcp_Report_App_View(void * buildfrom, int max_length);
    virtual ~Rtcp_Report_App_View();

    int get_length();
    void write_data(char* buf);

    virtual void debug_print();

    unsigned int get_subtype();
    unsigned int get_sender_ssrc();
    int get_sender_width();
    int get_sender_height();
private:
    unsigned sender_ssrc;
    int sender_width;
    int sender_height;
};

#endif // RTCP_REPORT_APP_VIEW_H
