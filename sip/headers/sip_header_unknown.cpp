#include "sip_header_unknown.h"

const std::string sipHeaderUnknownTypeString ="";

Sip_Header_Value_Unknown::Sip_Header_Value_Unknown(const std::string &headerName_, const std::string &build_from)
        : Sip_Header_Value_String(SIP_HEADER_TYPE_UNKNOWN, headerName_, build_from)
{
}
