#include "rtcp_report_reception_block.h"

#include<stdlib.h>
#include<iostream>
#include "my_assert.h"

using namespace std;

Rtcp_Report_Reception_Block::Rtcp_Report_Reception_Block(unsigned ssrc)
{
    this->ssrc = ssrc;
    this->fraction_lost=0;
    this->cumulative_n_lost = 0;
    this->seq_high = 0;
    this->jitter=0;
    this->last_sr=0;
    this->dlsr = 0;
}

Rtcp_Report_Reception_Block::Rtcp_Report_Reception_Block(void *build_from, int max_length)
{
    uint8_t * bytearray = (uint8_t *)build_from;
    if (max_length<24)
    {
        cerr << "ERROR: too short to parse reception block (int RtpReportReceptionBlock)"<<endl;
        exit(1);
    }

    this->ssrc = U32_AT( bytearray );
    this->fraction_lost = bytearray[4];
    this->cumulative_n_lost = U32_AT( bytearray + 4 ) && 0x00FFFFFF;

    this->seq_high = U32_AT( bytearray + 8 );
    this->jitter = U32_AT( bytearray + 9 );
    this->last_sr = U32_AT( bytearray + 10 );
    this->dlsr = U32_AT( bytearray + 11 );
}

int Rtcp_Report_Reception_Block::get_length()
{
    return 24;
}

void Rtcp_Report_Reception_Block::write_data(char* buf)
{
    uint32_t* lp=(uint32_t*)buf;
    byte_t* bp=(byte_t*)buf;
    lp[0]=hton32(ssrc);
    lp[1]=hton32(cumulative_n_lost);
    bp[4]=fraction_lost; //overwrite highest byte of lp[1]
    lp[2]=hton32(seq_high);
    lp[3]=hton32(jitter);
    lp[4]=hton32(last_sr);
    lp[5]=hton32(dlsr);
}

void Rtcp_Report_Reception_Block::debug_print()
{
    cerr << " rtcp report reception block: 0x"<< endl;
    cerr.setf(ios::hex, ios::basefield);
    cerr << "\tssrc: 0x"<<ssrc<< endl;
    cerr << "\tfraction lost: 0x"<<fraction_lost<< endl;
    cerr << "\tcumulative number lost: 0x"<< cumulative_n_lost << endl;
    cerr << "\tseq_high: 0x"<< seq_high<< endl;
    cerr << "\tjitter: 0x"<< jitter<< endl;
    cerr << "\tlast_sr: 0x"<< last_sr<< endl;
    cerr << "\tdlsr: 0x"<< dlsr<< endl;
    cerr.setf(ios::dec, ios::basefield);
}

void Rtcp_Report_Reception_Block::set_fraction_lost(unsigned n)
{
    fraction_lost = n;
}

unsigned Rtcp_Report_Reception_Block::get_fraction_lost()
{
    return fraction_lost;
}

void Rtcp_Report_Reception_Block::set_cumulative_nlost(unsigned n)
{
    cumulative_n_lost = n;
}

unsigned Rtcp_Report_Reception_Block::get_cumulative_nlost()
{
    return cumulative_n_lost;
}

void Rtcp_Report_Reception_Block::set_seq_high(unsigned i)
{
    seq_high = i;
}

unsigned Rtcp_Report_Reception_Block::get_seq_high()
{
    return seq_high;
}

void Rtcp_Report_Reception_Block::set_jitter(unsigned i)
{
    this->jitter = i;
}

unsigned Rtcp_Report_Reception_Block::get_jitter()
{
    return jitter;
}

void Rtcp_Report_Reception_Block::set_last_sr(unsigned i)
{
    last_sr = i;
}

unsigned Rtcp_Report_Reception_Block::get_last_sr()
{
    return last_sr;
}

void Rtcp_Report_Reception_Block::set_dlsr(unsigned i)
{
    dlsr = i;
}

unsigned Rtcp_Report_Reception_Block::get_dlsr()
{
    return dlsr;
}
