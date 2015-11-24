#ifndef RTCP_REPORT_XR_H
#define RTCP_REPORT_XR_H

#include <vector>

#include "rtcp_report.h"
#include "xr_report_block.h"

class Rtcp_Report_XR : public Rtcp_Report
{
public:
    Rtcp_Report_XR(void *build_from, int max_length);
    virtual ~Rtcp_Report_XR(){}

    virtual void debug_print();
    virtual int get_length();
    void write_data(char* buf);

private:
    unsigned ssrc_or_csrc;
    std::vector <XR_Report_Block *> xr_blocks;
};

#endif // RTCP_REPORT_XR_H
