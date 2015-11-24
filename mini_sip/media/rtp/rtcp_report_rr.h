#ifndef RTCP_REPORT_RR_H
#define RTCP_REPORT_RR_H
#include <vector>
#include "rtcp_report.h"
#include "rtcp_report_reception_block.h"

class Rtcp_Report_RR : public Rtcp_Report
{
public:
    Rtcp_Report_RR(unsigned sender_ssrc);

    Rtcp_Report_RR(void * build_from, int max_length);

    virtual ~Rtcp_Report_RR();

    int get_length();
    void write_data(char* buf);

    virtual void debug_print();

    int get_nreport_blocks();
    Rtcp_Report_Reception_Block &get_reception_block(int i);

    void add_reception_block(Rtcp_Report_Reception_Block block);

private:
    unsigned sender_ssrc;
    std::vector<Rtcp_Report_Reception_Block>reception_blocks;
};

#endif // RTCP_REPORT_RR_H
