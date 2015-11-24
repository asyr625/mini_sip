#include "sip_message_content_im.h"

SRef<Sip_Message_Content*> sip_im_message_content_factory(const std::string &buf, const std::string &)
{
    return new Sip_Message_Content_IM(buf);
}

Sip_Message_Content_IM::Sip_Message_Content_IM(std::string m)
    : _msg(m)
{
}

std::string Sip_Message_Content_IM::get_string() const
{
    return _msg;
}

std::string Sip_Message_Content_IM::get_content_type() const
{
    return "text/plain";
}
