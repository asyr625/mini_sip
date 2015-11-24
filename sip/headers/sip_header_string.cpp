#include "sip_header_string.h"

Sip_Header_Value_String::Sip_Header_Value_String(int type_, const std::string& typeStr, const std::string &build_from)
        : Sip_Header_Value(type_, typeStr)
{
    _str = build_from;
}

Sip_Header_Value_String::~Sip_Header_Value_String()
{
}

std::string Sip_Header_Value_String::get_string() const
{
    return _str;
}

void Sip_Header_Value_String::set_string(const std::string &newStr)
{
    _str = newStr;
}
