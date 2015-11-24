#include "sdp_header.h"


Sdp_Header::Sdp_Header(int type, int priority)
    : _string_representation_up2date(false), _type(type), _priority(priority)
{
}

Sdp_Header::Sdp_Header(int type, int priority, std::string value)
    : _string_representation_up2date(true),
      _string_representation(value),
      _type(type),
      _priority(priority)
{
}
