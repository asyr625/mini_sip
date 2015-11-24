#ifndef SDP_HEADERB_H
#define SDP_HEADERB_H

#include "sdp_header.h"

class Sdp_HeaderB : public Sdp_Header
{
public:
    Sdp_HeaderB(const std::string &buildFrom);
    virtual ~Sdp_HeaderB();

    virtual std::string get_mem_object_type() const { return "SdpHeaderB"; }

    unsigned int get_ct() const;
    void set_ct(const unsigned int &_ct);

    unsigned int get_as() const;
    void set_as(const unsigned int &_as);

    unsigned int get_tias() const;
    void set_tias(const unsigned int &_tias);

    unsigned int get_rs() const;
    void set_rs(const unsigned int &_rs);

    unsigned int get_rr() const;
    void set_rr(const unsigned int &_rr);

    virtual std::string get_string() const;
private:
    unsigned int ct, as, tias, rs, rr;
};

#endif // SDP_HEADERB_H
