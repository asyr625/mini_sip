#include<stdlib.h>
#include<iostream>

#include "xr_report_block.h"
#include "xr_voip_report_block.h"

XR_Report_Block *XR_Report_Block::build_from(void *from, int max_length)
{
    unsigned char *ucptr = (unsigned char *)from;
    switch(*ucptr){
        case LOSS_RLE_REPORT:
            std::cerr << "XR report type LOSS_RLE_REPORT not implemented"<< std::endl;
            exit(1);
            break;
        case DUPLICATE_RLE_REPORT:
            std::cerr << "XR report type DUPLICATE_RLE_REPORT not implemented"<< std::endl;
            exit(1);
            break;
        case TIMESTAMP_REPORT:
            std::cerr << "XR report type TIMESTAMP_REPORT not implemented"<< std::endl;
            exit(1);
            break;
        case STATISTIC_SUMMARY_REPORT:
            std::cerr << "XR report type STATISTIC_SUMMARY_REPORT not implemented"<< std::endl;
            exit(1);
            break;
        case RECEIVER_TIMESTAMP_REPORT:
            std::cerr << "XR report type RECEIVER_TIMESTAMP_REPORT not implemented"<< std::endl;
            exit(1);
            break;
        case DLRR_REPORT:
            std::cerr << "XR report type DLRR_REPORT not implemented"<< std::endl;
            exit(1);
            break;
        case VOIP_METRICS_REPORT:
            return new XR_VoIP_Report_Block(from, max_length);
            break;
        default:
            std::cerr << "ERROR: Unknown RTCP XR Report block"<< std::endl;
            exit(1);
    }

    return NULL;
}

void XR_Report_Block::parse_header(void *from)
{
    unsigned char *ucptr = (unsigned char *)from;
    block_type = *ucptr;
    type_specific = *(ucptr+1);
    block_length = U16_AT( (unsigned short *)from );
}
