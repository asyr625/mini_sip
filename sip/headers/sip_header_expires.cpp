#include <iostream>

#include "sip_header_expires.h"
#include "my_error.h"
#include "string_utils.h"


SRef<Sip_Header_Value *> expiresFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Expires(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_expires_factory = expiresFactory;

const std::string sipHeaderValueExpiresTypeStr = "Expires";

Sip_Header_Value_Expires::Sip_Header_Value_Expires(const std::string &build_from)
    : Sip_Header_Value(SIP_HEADER_TYPE_EXPIRES,sipHeaderValueExpiresTypeStr)
{
    std::string tmp = trim(build_from);
    char *endptr;
    const char *s = tmp.c_str();
    int n = strtol(s, &endptr, 10);
    if( *endptr == 0 )
    {
        _timeout = n;
    }
    else
    {
        std::cerr << "WARNING: Could not parse Expires header - setting to 300 instead"<<std::endl;
        _timeout=300;
    }
}

Sip_Header_Value_Expires::Sip_Header_Value_Expires(int n)
    : Sip_Header_Value(SIP_HEADER_TYPE_EXPIRES,sipHeaderValueExpiresTypeStr)
{
    _timeout = n;
}



Sip_Header_Value_Expires::~Sip_Header_Value_Expires()
{
}


std::string Sip_Header_Value_Expires::get_string() const
{
    return itoa(_timeout);
}

int32_t Sip_Header_Value_Expires::get_timeout() const
{
    return _timeout;
}
