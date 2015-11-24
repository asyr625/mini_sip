#ifndef RTCP_REPORT_SENDER_INFO_H
#define RTCP_REPORT_SENDER_INFO_H

#include "my_types.h"
#include "sobject.h"

#include "mini_defines.h"
#include <iostream>
using namespace std;

class Rtcp_Report_Sender_Info : public SObject
{
public:
    Rtcp_Report_Sender_Info(void *buildfrom, int max_length);

    int get_length();
    void write_data(char* buf);

    void debug_print();

    void set_ntp_timestamp_msw(unsigned t);
    unsigned get_ntp_timestamp_msw();

    void set_ntp_timestamp_lsw(unsigned t);
    unsigned get_ntp_timestamp_lsw();

    void set_ntp_timestamp(const long double &secondsSinceJan1st1900)
    {
        uint64_t integralSecondsSinceJan1st1900 = secondsSinceJan1st1900;
        set_ntp_timestamp_msw(integralSecondsSinceJan1st1900);
        set_ntp_timestamp_lsw((secondsSinceJan1st1900 - integralSecondsSinceJan1st1900) * (uint64_t(1) << 32));
    }
    long double get_ntp_timestamp_secs_since_jan1st1900()
    {
        return get_ntp_timestamp_msw() + ((long double)get_ntp_timestamp_lsw()) / (uint64_t(1) << 32);
    }
    int64_t get_ntp_timestamp_usecs_since_jan1st1970()
    {
        return (get_ntp_timestamp_secs_since_jan1st1900() - ((long double)25567) * 24 * 60 * 60) * 1000000 + 0.499999;
    }

    void set_rtp_timestamp(unsigned t);
    unsigned get_rtp_timestamp();

    void set_sender_packet_count(int i);
    unsigned get_sender_packet_count();

    void set_sender_octet_count(int i);
    unsigned get_sender_octet_count();

private:
    unsigned ntp_msw;
    unsigned ntp_lsw;
    unsigned rtp_timestamp;
    unsigned sender_packet_count;
    unsigned sender_octet_count;
};

#endif // RTCP_REPORT_SENDER_INFO_H
