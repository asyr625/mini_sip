#include <iostream>
#include "my_assert.h"
#include "xr_voip_report_block.h"

XR_VoIP_Report_Block::XR_VoIP_Report_Block(void *build_from, int max_length)
{
    uint8_t * bytearray = (uint8_t *)build_from;
    my_assert(bytearray[1]==0);
    my_assert(max_length>=7);

    //this->block_type = vmrp->block_type;
    this->block_type = bytearray[0];
    my_assert(this->block_type==VOIP_METRICS_REPORT);
    //this->block_length = ntohs(vmrp->block_length);
    this->block_length = U16_AT( bytearray + 2 );
    my_assert(this->block_length==6);
    //this->loss_rate = vmrp->loss_rate;
    this->loss_rate = bytearray[4];
    //this->discard_rate = vmrp->discard_rate;
    this->discard_rate = bytearray[5];
    //this->burst_density = vmrp->burst_density;
    this->burst_density = bytearray[6];
    this->gap_density = bytearray[7];
    //this->gap_density = vmrp->gap_density;
    //this->burst_duration = ntohs(vmrp->burst_duration);
    this->burst_duration = U16_AT( bytearray + 8 );
    //this->gap_duration = ntohs(vmrp->gap_duration);
    this->gap_duration = U16_AT( bytearray + 10 );
    //this->round_trip_delay = ntohs(vmrp->round_trip_delay);
    this->round_trip_delay = U16_AT( bytearray + 12 );
    //this->end_system_delay = ntohs(vmrp->end_system_delay);
    this->end_system_delay = U16_AT( bytearray + 14 );
    //this->signal_power = vmrp->signal_power;
    this->signal_power = bytearray[16];
    //this->RERL = vmrp->RERL;
    this->RERL = bytearray[17];
    //this->noise_level = vmrp->noise_level;
    this->noise_level = bytearray[18];
    //this->Gmin = vmrp->Gmin;
    this->Gmin = bytearray[19];
    //this->R_factor = vmrp->R_factor;
    this->R_factor = bytearray[20];
    //this->ext_R_factor = vmrp->ext_R_factor;
    this->ext_R_factor = bytearray[21];
    //this->MOS_LQ = vmrp->MOS_LQ;
    this->MOS_LQ = bytearray[22];
    //this->MOS_CQ = vmrp->MOS_CQ;
    this->MOS_CQ = bytearray[23];
    //this->RX_config = vmrp->RX_config;
    this->RX_config = bytearray[24];
    //this->JB_nominal = vmrp->JB_nominal;
    this->JB_nominal = bytearray[25];
    //this->JB_maximum = vmrp->JB_maximum;
    this->JB_maximum = bytearray[26];
    //this->JB_abs_max = vmrp->JB_abs_max;
    this->JB_abs_max = bytearray[27];
}

void XR_VoIP_Report_Block::debug_print()
{
    std::cerr.setf( std::ios::hex, std::ios::basefield );
    std::cerr <<"\tblock_type=0x"<<this->block_type<<std::endl;
    std::cerr <<"\tblock_length=0x"<< this->block_length << std::endl;
    std::cerr <<"\tloss_rate=0x"<<this->loss_rate << std::endl;
    std::cerr <<"\tdiscard_rate=0x"<<this->discard_rate << std::endl;
    std::cerr <<"\tburst_density=0x"<<this->burst_density << std::endl;
    std::cerr <<"\tgap_density=0x"<<this->gap_density << std::endl;
    std::cerr <<"\tburst_duration=0x"<<this->burst_duration << std::endl;
    std::cerr <<"\tgap_duration=0x"<<this->gap_duration << std::endl;
    std::cerr <<"\tround_trip_delay=0x"<<this->round_trip_delay << std::endl;
    std::cerr <<"\tend_system_delay=0x"<<this->end_system_delay << std::endl;
    std::cerr <<"\tsignal_power=0x"<<this->signal_power << std::endl;
    std::cerr <<"\tRERL=0x"<<this->RERL << std::endl;
    std::cerr <<"\tnoise_level=0x"<<this->noise_level <<std::endl;
    std::cerr <<"\tGmin=0x"<<this->Gmin << std::endl;
    std::cerr <<"\tR_factor=0x"<<this->R_factor << std::endl;
    std::cerr <<"\text_R_factor=0x"<<this->ext_R_factor << std::endl;
    std::cerr <<"\tMOS_LQ=0x"<<this->MOS_LQ << std::endl;
    std::cerr <<"\tMOS_CQ=0x"<<this->MOS_CQ << std::endl;
    std::cerr <<"\tRX_config=0x"<<this->RX_config << std::endl;
    std::cerr <<"\tJB_nominal=0x"<<this->JB_nominal << std::endl;
    std::cerr <<"\tJB_maximum=0x"<<this->JB_maximum << std::endl;
    std::cerr <<"\tJB_abs_max=0x"<<this->JB_abs_max << std::endl;

    std::cerr.setf( std::ios::dec, std::ios::basefield );
}

int XR_VoIP_Report_Block::size()
{
    return 28;
}
