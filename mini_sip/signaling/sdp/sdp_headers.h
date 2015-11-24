#ifndef SDP_HEADERS_H
#define SDP_HEADERS_H

#include "sdp_header.h"

class Sdp_HeaderS : public Sdp_Header{
public:
    Sdp_HeaderS(std::string buildFrom);
    virtual ~Sdp_HeaderS();

    virtual std::string get_mem_object_type() const {return "SdpHeaderS";}

    std::string get_session_name() const;
    void set_session_name(std::string s);

    virtual std::string get_string() const;

private:
    std::string _session_name;
};

#endif // SDP_HEADERS_H
