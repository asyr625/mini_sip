#include "sdp_headeri.h"
#include "string_utils.h"
#include <stdlib.h>
#include <iostream>
using namespace std;

Sdp_HeaderI::Sdp_HeaderI(std::string buildFrom)
    :Sdp_Header(SDP_HEADER_TYPE_I, 9, buildFrom)
{
    my_assert(buildFrom.substr(0,2) == "i=");
    _session_information = trim(buildFrom.substr(2, buildFrom.length()-2));
}

Sdp_HeaderI::~Sdp_HeaderI()
{
}

std::string Sdp_HeaderI::get_session_information() const
{
    return _session_information;
}

void Sdp_HeaderI::set_session_information(string s)
{
    this->_session_information = s;
}

std::string Sdp_HeaderI::get_string() const
{
    if( _string_representation_up2date )
        return _string_representation;

    return "i=" + _session_information;
}
