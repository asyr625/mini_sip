#include "sdp_headerv.h"

#include <stdlib.h>
#include "string_utils.h"
Sdp_HeaderV::Sdp_HeaderV(std::string buildFrom)
    :Sdp_Header(SDP_HEADER_TYPE_V, 1, buildFrom)
{
    _v = atoi( buildFrom.c_str() );
}

Sdp_HeaderV::Sdp_HeaderV(int32_t ver)
    :Sdp_Header(SDP_HEADER_TYPE_V, 1)
{
    _v = ver;
}


int32_t Sdp_HeaderV::get_version() const
{
    return _v;
}

void Sdp_HeaderV::set_version(int32_t ver)
{
    _string_representation_up2date = false;
    _v = ver;
}

std::string Sdp_HeaderV::get_string() const
{
    if( _string_representation_up2date )
        return _string_representation;

    return "v=" + itoa(_v);
}
