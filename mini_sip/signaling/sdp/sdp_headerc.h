#ifndef SDP_HEADERC_H
#define SDP_HEADERC_H
#include "sdp_header.h"
#include "ipaddress.h"

class Sdp_HeaderC : public Sdp_Header
{
public:
    Sdp_HeaderC(std::string buildFrom);
    Sdp_HeaderC(std::string netType, std::string addrType, std::string addr);
    virtual ~Sdp_HeaderC();

    virtual std::string get_mem_object_type() const {return "SdpHeaderC";}

    const std::string &get_net_type() const;
    void set_net_type(std::string netType);

    const std::string &get_addr_type() const;
    void set_addr_type(std::string addrType);

    const std::string &get_addr() const;
    void set_addr(std::string addr);

    virtual std::string get_string() const;

    SRef<IPAddress*> get_ipadress();

private:
    std::string _net_type;
    std::string _addr_type;
    std::string _addr;

    SRef<IPAddress*> _ipAddr;
};

#endif // SDP_HEADERC_H
