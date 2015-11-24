#ifndef SDP_HEADERI_H
#define SDP_HEADERI_H

#include "sdp_header.h"

class Sdp_HeaderI : public Sdp_Header
{
public:
    Sdp_HeaderI(std::string buildFrom);
    virtual ~Sdp_HeaderI();

    virtual std::string get_mem_object_type() const {return "SdpHeaderI";}

    std::string get_session_information() const;
    void set_session_information(std::string s);

    virtual  std::string get_string() const;

private:
    std::string _session_information;
};

#endif // SDP_HEADERI_H
