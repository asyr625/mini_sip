#ifndef RTCP_REPORT_FIR_H
#define RTCP_REPORT_FIR_H
#include "my_types.h"
#include "rtcp_report.h"

class Rtcp_Report_Fir : public Rtcp_Report
{
public:
    Rtcp_Report_Fir(uint32_t ssrc);
    Rtcp_Report_Fir(void* build_from, int max_length);
    virtual ~Rtcp_Report_Fir();

    int get_length();
    void write_data(char* buf);

    virtual void debug_print();

    uint32_t get_ssrc();
private:
    uint32_t ssrc;
};

#endif // RTCP_REPORT_FIR_H
