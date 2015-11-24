#ifndef SDP_HEADERV_H
#define SDP_HEADERV_H

#include "sdp_header.h"

class Sdp_HeaderV : public Sdp_Header
{
public:
    Sdp_HeaderV(std::string buildFrom);
    Sdp_HeaderV(int32_t ver);

    virtual std::string get_mem_object_type() const {return "SdpHeaderV";}

    int32_t get_version() const;
    void set_version(int32_t ver);

    virtual std::string get_string() const;

private:
    int32_t _v;
};

#endif // SDP_HEADERV_H
