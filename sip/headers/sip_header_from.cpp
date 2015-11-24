#include "sip_header_from.h"


SRef<Sip_Header_Value *> fromFactory(const std::string &build_from)
{
    return new Sip_Header_Value_From(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_from_factory = fromFactory;

const std::string sipHeaderValueFrom = "From";

Sip_Header_Value_From::Sip_Header_Value_From(const std::string &build_from)
        : Sip_Header_Value(SIP_HEADER_TYPE_FROM,sipHeaderValueFrom),
          _uri(build_from)
{
}

Sip_Header_Value_From::Sip_Header_Value_From(const Sip_Uri& u)
        : Sip_Header_Value(SIP_HEADER_TYPE_FROM, sipHeaderValueFrom),
          _uri(u)
{
}

Sip_Header_Value_From::~Sip_Header_Value_From()
{
}

std::string Sip_Header_Value_From::get_string() const
{
    return _uri.get_string();
}

Sip_Uri Sip_Header_Value_From::get_uri() const
{
    return _uri;
}

void Sip_Header_Value_From::set_uri(const Sip_Uri &uri)
{
    this->_uri = uri;
}
