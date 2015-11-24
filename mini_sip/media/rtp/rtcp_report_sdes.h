#ifndef RTCP_REPORT_SDES_H
#define RTCP_REPORT_SDES_H

#include "rtcp_report.h"
#include "sdes_chunk.h"

class Rtcp_Report_Sdes : public Rtcp_Report
{
public:
    Rtcp_Report_Sdes(Sdes_Chunk*chunk);
    Rtcp_Report_Sdes(void * build_from, int max_length);
    virtual ~Rtcp_Report_Sdes();
    int get_length();
    void write_data(char* buf);

    virtual void debug_print();

    Sdes_Item* get_item(int type, uint32_t* ssrc_out);
private:
    std::vector<Sdes_Chunk*>chunks;
};

#endif // RTCP_REPORT_SDES_H
