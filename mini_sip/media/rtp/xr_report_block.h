#ifndef XR_REPORT_BLOCK_H
#define XR_REPORT_BLOCK_H

#include "mini_defines.h"

#define LOSS_RLE_REPORT            1
#define DUPLICATE_RLE_REPORT       2
#define TIMESTAMP_REPORT           3
#define STATISTIC_SUMMARY_REPORT   4
#define RECEIVER_TIMESTAMP_REPORT  5
#define DLRR_REPORT                6
#define VOIP_METRICS_REPORT        7

class XR_Report_Block
{
public:
    virtual ~XR_Report_Block(){}
    static XR_Report_Block *build_from(void *from, int max_length);

    virtual void debug_print() = 0;
    virtual int size() = 0;

protected:
    void parse_header(void *from);

    unsigned block_type;
    unsigned type_specific;
    unsigned block_length;
};

#endif // XR_REPORT_BLOCK_H
