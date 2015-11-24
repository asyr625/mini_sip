#include "sip_header_content_length.h"
#include "string_utils.h"
#include "sip_exception.h"

SRef<Sip_Header_Value *> contentlengthFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Content_Length(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_content_length_factory = contentlengthFactory;

const std::string sipHeaderValueContentLengthTypeStr = "Content-Length";

Sip_Header_Value_Content_Length::Sip_Header_Value_Content_Length(const std::string &build_from)
    : Sip_Header_Value(SIP_HEADER_TYPE_CONTENTLENGTH,sipHeaderValueContentLengthTypeStr)
{
    unsigned i=0;
    unsigned len = (unsigned)build_from.length();
    while (is_ws(build_from[i]))
        i++;

    std::string num = "";
    while (build_from[i]>='0' && build_from[i]<='9' && !(i>=len) )
    {
        num += build_from[i];
        i++;
    }

    while ( i < len && is_ws(build_from[i]) )
        i++;

    if (num.size() == 0 || i < len )
    {
        throw Sip_Exception_Invalid_Message("SipHeaderValueContentLength malformed");
    }

    _content_length = atoi(num.c_str());
}

Sip_Header_Value_Content_Length::Sip_Header_Value_Content_Length(int32_t length)
    : Sip_Header_Value(SIP_HEADER_TYPE_CONTENTLENGTH,sipHeaderValueContentLengthTypeStr),
      _content_length (length)
{
}

Sip_Header_Value_Content_Length::~Sip_Header_Value_Content_Length()
{
}


std::string Sip_Header_Value_Content_Length::get_string() const
{
    return itoa(_content_length);
}

int32_t Sip_Header_Value_Content_Length::get_content_length() const
{
    return _content_length;
}

void Sip_Header_Value_Content_Length::set_content_length(int32_t content_length)
{
    _content_length = content_length;
}
