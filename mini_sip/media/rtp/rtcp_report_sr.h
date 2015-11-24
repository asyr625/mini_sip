#ifndef RTCP_REPORT_SR_H
#define RTCP_REPORT_SR_H
#include <vector>

#include "sobject.h"
#include "rtcp_report.h"
#include "rtcp_report_reception_block.h"
#include "rtcp_report_sender_info.h"

class Rtcp_Report_SR : public Rtcp_Report
{
public:
    Rtcp_Report_SR(unsigned ssrc);

    Rtcp_Report_SR(void * buildfrom, int max_length);
    virtual ~Rtcp_Report_SR();

    virtual int get_length();
    void write_data(char* buf);

    virtual void debug_print();

    SRef<Rtcp_Report_Sender_Info *> get_sender_info();
    void set_sender_info(const Rtcp_Report_Sender_Info &info);

    int get_nreport_blocks();

    Rtcp_Report_Reception_Block &get_reception_block(int i);
    void add_reception_block(Rtcp_Report_Reception_Block &b);

    unsigned int get_sender_ssrc();

private:
    unsigned sender_ssrc;
    SRef<Rtcp_Report_Sender_Info *> sender_info;
    std::vector<Rtcp_Report_Reception_Block>reception_blocks;
};

#endif // RTCP_REPORT_SR_H
