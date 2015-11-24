#ifndef SDP_HEADERA_H
#define SDP_HEADERA_H

#include "sdp_header.h"

class Sdp_HeaderA : public Sdp_Header
{
public:
    Sdp_HeaderA(std::string buildFrom);
    virtual ~Sdp_HeaderA();

    virtual std::string get_mem_object_type() const {return "SdpHeaderA";}

    std::string get_attributes() const;
    void set_attributes(std::string attr);

    virtual std::string get_string() const;

    std::string get_attribute_type() const;
    std::string get_attribute_value() const;

    void get_att_from_file_selector();

    std::string get_rtp_map(int format) const;

    bool name;
    bool type;
    bool size;
    bool hash;

    std::string filename;
    std::string filetype;
    std::string filesizes;
    std::string hashused;
    std::string hashforfile;

private:
    std::string _attributes;
};

#endif // SDP_HEADERA_H
