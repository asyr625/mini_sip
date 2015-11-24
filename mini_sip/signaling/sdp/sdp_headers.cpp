#include "sdp_headers.h"
#include "string_utils.h"

Sdp_HeaderS::Sdp_HeaderS(std::string buildFrom)
    :Sdp_Header(SDP_HEADER_TYPE_S, 3, buildFrom)
{
    my_assert(buildFrom.substr(0,2)=="s=");
    _session_name = trim(buildFrom.substr(2, buildFrom.length()-2));
}
Sdp_HeaderS::~Sdp_HeaderS()
{
}

std::string Sdp_HeaderS::get_session_name() const
{
    return _session_name;
}

void Sdp_HeaderS::set_session_name(std::string s)
{
    _string_representation_up2date = false;
    _session_name = s;
}

std::string Sdp_HeaderS::get_string() const
{
    if( _string_representation_up2date )
        return _string_representation;

    return "s=" + _session_name;
}
