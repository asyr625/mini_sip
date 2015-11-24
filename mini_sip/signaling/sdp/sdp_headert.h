#ifndef SDP_HEADERT_H
#define SDP_HEADERT_H

#include "sdp_header.h"

class Sdp_HeaderT : public Sdp_Header
{
public:
    Sdp_HeaderT(std::string buildFrom);
    Sdp_HeaderT(int32_t start_time, int32_t stop_time);
    ~Sdp_HeaderT();

    virtual std::string get_mem_object_type() const {return "SdpHeaderT";}

    int32_t get_start_time() const;
    void set_start_time(int32_t time);

    int32_t get_stop_time() const;
    void set_stop_time(int32_t time);

    virtual std::string get_string() const;

private:
    int32_t _start_time;
    int32_t _stop_time;
};

#endif // SDP_HEADERT_H
