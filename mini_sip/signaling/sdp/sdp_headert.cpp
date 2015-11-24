#include "sdp_headert.h"

#include <stdlib.h>
#include "string_utils.h"

Sdp_HeaderT::Sdp_HeaderT(std::string buildFrom)
    : Sdp_Header(SDP_HEADER_TYPE_T, 6, buildFrom)
{
        size_t len = buildFrom.length();
        if (buildFrom.substr(0,2)!="t=")
        {
    #ifdef DEBUG_OUTPUT
            std::cerr << "ERROR: Origin sdp header is not starting with <o=>"<< std::endl;
    #endif
        }

        size_t i=2;
        while ( buildFrom[i]==' ' && i<len )
            i++;

        std::string startstr="";
        while ( buildFrom[i]!=' ' && i<len )
            startstr += buildFrom[i++];

        while ( buildFrom[i]==' ' && i<len )
            i++;

        std::string stopstr="";
        while ( buildFrom[i]!=' ' && i<len )
            stopstr += buildFrom[i++];

        _start_time = atoi(startstr.c_str());
        _stop_time  = atoi(stopstr.c_str());
}

Sdp_HeaderT::Sdp_HeaderT(int32_t start_time, int32_t stop_time)
    : Sdp_Header(SDP_HEADER_TYPE_T, 6)
{
    this->_start_time = start_time;
    this->_stop_time = stop_time;
}

Sdp_HeaderT::~Sdp_HeaderT()
{
}


int32_t Sdp_HeaderT::get_start_time() const
{
    return _start_time;
}

void Sdp_HeaderT::set_start_time(int32_t time)
{
    _string_representation_up2date = false;
    _start_time = time;
}

int32_t Sdp_HeaderT::get_stop_time() const
{
    return _stop_time;
}

void Sdp_HeaderT::set_stop_time(int32_t time)
{
    _string_representation_up2date = false;
    _stop_time = time;
}

std::string Sdp_HeaderT::get_string() const
{
    if( _string_representation_up2date )
        return _string_representation;

    return "t=" +itoa(_start_time)+" "+itoa(_stop_time);
}
