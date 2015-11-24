#ifndef RTCP_REPORT_RECEPTION_BLOCK_H
#define RTCP_REPORT_RECEPTION_BLOCK_H

class Rtcp_Report_Reception_Block
{
public:
    Rtcp_Report_Reception_Block(unsigned ssrc);

    Rtcp_Report_Reception_Block(void *build_from, int max_length);
    int get_length();
    void write_data(char* buf);
    void debug_print();

    void set_fraction_lost(unsigned n);
    unsigned get_fraction_lost();

    void set_cumulative_nlost(unsigned n);
    unsigned get_cumulative_nlost();

    void set_seq_high(unsigned i);
    unsigned get_seq_high();

    void set_jitter(unsigned i);
    unsigned get_jitter();

    void set_last_sr(unsigned i);
    unsigned get_last_sr();

    void set_dlsr(unsigned i);
    unsigned get_dlsr();

private:
    unsigned ssrc;
    unsigned fraction_lost;
    unsigned cumulative_n_lost;
    unsigned seq_high;
    unsigned jitter;
    unsigned last_sr;
    unsigned dlsr;
};

#endif // RTCP_REPORT_RECEPTION_BLOCK_H
