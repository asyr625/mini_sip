#include "sip_message_content_unknown.h"

SRef<Sip_Message_Content*> sip_unknown_message_content_factory(const std::string &buf, const std::string &type)
{
    return new Sip_Message_Content_Unknown(buf,type);
}

Sip_Message_Content_Unknown::Sip_Message_Content_Unknown(std::string m, std::string contentType)
    : _msg(m), _content_type(contentType)
{
}


std::string Sip_Message_Content_Unknown::get_string() const
{
    return _msg;
}

std::string Sip_Message_Content_Unknown::get_content_type() const
{
    return _content_type;
}
