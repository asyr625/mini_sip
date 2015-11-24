#ifndef SDP_HEADERO_H
#define SDP_HEADERO_H

#include "sdp_header.h"

class Sdp_HeaderO : public Sdp_Header
{
public:
    Sdp_HeaderO(std::string buildFrom);
    Sdp_HeaderO(std::string username_, std::string session_id_, std::string version_,
                std::string net_type_, std::string addr_type_, std::string addr_);
    virtual ~Sdp_HeaderO();

    virtual std::string get_mem_object_type() const {return "SdpHeaderO";}

    std::string get_username() const;
    void set_username(std::string username);

    std::string get_session_id() const;
    void set_session_id(std::string sess_id);

    std::string get_version() const;
    void set_version(std::string version);

    std::string get_net_type() const;
    void set_net_type(std::string netType);

    std::string get_addr_type() const;
    void set_addr_type(std::string addrType);

    std::string get_addr() const;
    void set_addr(std::string addr);

    virtual std::string get_string() const;

private:
    std::string username;
    std::string session_id;
    std::string version;
    std::string net_type;
    std::string addr_type;
    std::string addr;
};

#endif // SDP_HEADERO_H
