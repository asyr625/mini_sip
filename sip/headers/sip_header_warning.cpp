#include "sip_header_warning.h"
#include "string_utils.h"

SRef<Sip_Header_Value *> warningFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Warning(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_warning_factory = warningFactory;

const std::string sipHeaderValueWarningTypeStr = "Warning";

Sip_Header_Value_Warning::Sip_Header_Value_Warning(const std::string &build_from)
    : Sip_Header_Value(SIP_HEADER_TYPE_WARNING,sipHeaderValueWarningTypeStr)
{
    _error_code = 0;
    size_t blank1Pos = build_from.find(" ");
    size_t blank2Pos;
    if( blank1Pos == std::string::npos)
    {
        return;
    }

    blank2Pos = blank1Pos + 1 + build_from.substr(blank1Pos+1, std::string::npos).find(" ");
    if( blank2Pos == std::string::npos)
    {
        return;
    }

    _error_code = atoi(build_from.substr(0,3).c_str());
    _domain_name = build_from.substr(blank1Pos+1, blank2Pos-blank1Pos);
    _warning = build_from.substr(blank2Pos);
}

Sip_Header_Value_Warning::Sip_Header_Value_Warning(std::string domainName, uint16_t errorCode, std::string warning)
    : Sip_Header_Value(SIP_HEADER_TYPE_WARNING,sipHeaderValueWarningTypeStr)
{
    this->_error_code = errorCode;
    this->_domain_name = domainName;
    this->_warning = warning;
}

Sip_Header_Value_Warning::~Sip_Header_Value_Warning()
{
}


std::string Sip_Header_Value_Warning::get_string() const
{
    return /*"Warning: "+*/ itoa(_error_code)+" "+ _domain_name +" \""+ _warning +"\"";
}
std::string Sip_Header_Value_Warning::get_warning() const
{
    return _warning;
}

void Sip_Header_Value_Warning::set_warning(const std::string &ua)
{
    this->_warning = ua;
}

uint16_t Sip_Header_Value_Warning::get_error_code() const
{
    return _error_code;
}

void Sip_Header_Value_Warning::set_error_code(const uint16_t &errorCodec)
{
    this->_error_code = errorCodec;
}

std::string Sip_Header_Value_Warning::get_domain_name() const
{
    return _domain_name;
}

void Sip_Header_Value_Warning::set_domain_name(const std::string &domainName)
{
    this->_domain_name = domainName;
}
