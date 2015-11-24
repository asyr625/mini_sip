#include "sdp_headerc.h"

#include <stdlib.h>
#include <iostream>
using namespace std;

Sdp_HeaderC::Sdp_HeaderC(std::string buildFrom)
    : Sdp_Header( SDP_HEADER_TYPE_C, 4, buildFrom )
{
    size_t len = buildFrom.length();
    if (buildFrom.substr(0,2)!="c=")
    {
#ifdef DEBUG_OUTPUT
        std::cerr << "ERROR: Contact sdp header is not starting with <c=>"<< std::endl;
#endif
    }
    size_t i=2;
    while ( buildFrom[i]==' ' && i<len )
        i++;

    _net_type="";
    while ( buildFrom[i]!=' ' && i<len )
        _net_type += buildFrom[i++];

    while ( buildFrom[i]==' ' && i<len )
        i++;

    _addr_type="";
    while ( buildFrom[i]!=' ' && i<len )
        _addr_type += buildFrom[i++];

    while ( buildFrom[i]==' ' && i<len )
        i++;

    _addr="";
    while (buildFrom[i]!=' ' && i<len)
        _addr += buildFrom[i++];

}

Sdp_HeaderC::Sdp_HeaderC(std::string net_type, std::string addr_type, std::string addr_)
    :Sdp_Header(SDP_HEADER_TYPE_C, 4)
{
    this->_net_type = net_type;
    this->_addr_type = addr_type;
    this->_addr = addr_;
}

Sdp_HeaderC::~Sdp_HeaderC()
{
}

const std::string &Sdp_HeaderC::get_net_type()const
{
    return _net_type;
}

void Sdp_HeaderC::set_net_type(string net_type)
{
    _string_representation_up2date = false;
    this->_net_type = net_type;
}

const std::string &Sdp_HeaderC::get_addr_type()const
{
    return _addr_type;
}
void Sdp_HeaderC::set_addr_type(string addr_type)
{
    _string_representation_up2date = false;
    this->_addr_type = addr_type;
}

const std::string &Sdp_HeaderC::get_addr()const
{
    return _addr;
}

void Sdp_HeaderC::set_addr(string a)
{
    _string_representation_up2date=false;
    this->_addr = a;
}

std::string Sdp_HeaderC::get_string() const
{
    std::string ret="c=";
    if (_net_type=="")
        ret+="-";
    else
        ret+=_net_type;
    ret+=" ";
    if (_addr_type=="")
        ret+="-";
    else
        ret+=_addr_type;
    ret+=" ";
    if (_addr=="")
        ret+="-";
    else
        ret+=_addr;
    return ret;
}

SRef<IPAddress*> Sdp_HeaderC::get_ipadress()
{
    if( !_ipAddr )
    {
        if( _net_type != "IN" )
            return NULL;

        if( _addr_type != "IP4" && _addr_type != "IP6" )
            return NULL;

        _ipAddr = IPAddress::create( _addr, _addr_type == "IP6" );
    }

    return _ipAddr;
}
