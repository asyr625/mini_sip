#ifndef XR_VOIP_REPORT_BLOCK_H
#define XR_VOIP_REPORT_BLOCK_H

#include "xr_report_block.h"

class XR_VoIP_Report_Block : public XR_Report_Block
{
public:
    XR_VoIP_Report_Block(void *build_from, int max_length);

    virtual void debug_print();
    virtual int size();
private:
    unsigned block_type;
    unsigned reserved;
    unsigned block_length;

    unsigned loss_rate;
    unsigned discard_rate;
    unsigned burst_density;
    unsigned gap_density;

    unsigned burst_duration;
    unsigned gap_duration;

    unsigned round_trip_delay;
    unsigned end_system_delay;

    unsigned signal_power;
    unsigned RERL;
    unsigned noise_level;
    unsigned Gmin;

    unsigned R_factor;
    unsigned ext_R_factor;
    unsigned MOS_LQ;
    unsigned MOS_CQ;

    unsigned RX_config;
    unsigned JB_nominal;
    unsigned JB_maximum;
    unsigned JB_abs_max;
};

#endif // XR_VOIP_REPORT_BLOCK_H
