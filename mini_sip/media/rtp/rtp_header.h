#ifndef RTP_HEADER_H
#define RTP_HEADER_H

#include "my_types.h"
#include <vector>

class Rtp_Header
{
public:
    Rtp_Header();
    void set_version(int v);
    void set_extension(int x);
    int get_extension();
    void set_csrc_count(int cc);

    void set_marker(int m);
    bool get_marker();

    void set_payload_type(int pt);
    int get_payload_type();

    void set_seq_no(uint16_t seq_no);
    uint16_t get_seq_no();

    void set_timestamp(uint32_t t);
    uint32_t get_timestamp();

    void set_ssrc(uint32_t ssrc);
    uint32_t get_ssrc();

    void add_ssrc(int c);

#ifdef TCP_FRIENDLY
    void set_rtt_estimate(uint32_t rtt) { tcp_friendly_mode = true; rttestimate = rtt; }
    uint32_t get_rtt_estimate() { return rttestimate; }

    void set_sending_timestamp(uint32_t ts) { tcp_friendly_mode = true; sending_timestamp = ts; }
    uint32_t get_sending_timestamp() { return sending_timestamp; }
#endif

    void print_debug();

    int size();
    char *get_bytes();

    int csrc_count;
    int version;
    int extension;
    int marker;
    int payload_type;
    uint16_t sequence_number;
    uint32_t timestamp;
    uint32_t ssrc;
    std::vector<int> csrc;
private:
#ifdef TCP_FRIENDLY
    //code to support the variable bandwidth experiments in the
    //thesis by David Tlahuetl
    bool tcp_friendly_mode;
    uint32_t sending_timestamp;
    uint32_t rttestimate;
#endif
};

#endif // RTP_HEADER_H
