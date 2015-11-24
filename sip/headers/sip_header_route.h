#ifndef SIP_HEADER_ROUTE_H
#define SIP_HEADER_ROUTE_H
#include <list>
#include "sip_header.h"

extern Sip_Header_Factory_Func_Ptr sip_header_route_factory;

class Sip_Header_Value_Route : public Sip_Header_Value
{
public:
    Sip_Header_Value_Route(std::list<std::string> &routeSet);
    Sip_Header_Value_Route(const std::string &build_from);

    virtual ~Sip_Header_Value_Route();

    virtual std::string get_mem_object_type() const { return "SipHeaderRoute"; }
    std::string get_string() const;

    std::string get_route() const;

private:
    std::string _route;
};

#endif // SIP_HEADER_ROUTE_H
