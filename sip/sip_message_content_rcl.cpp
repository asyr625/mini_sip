#include "sip_message_content_rcl.h"
#include "string_utils.h"

SRef<Sip_Message_Content*> sip_rcl_content_factory(const std::string & buf, const std::string & ContentType)
{
    return new Sip_Message_Content_RCL(buf, ContentType);
}

Sip_Message_Content_RCL::Sip_Message_Content_RCL(std::string t)
    : _content_type(t)
{
}

Sip_Message_Content_RCL::Sip_Message_Content_RCL(std::string content, std::string t)
{
    this->_content_type = t;
    this->_participants = split(content, true, ',', false);
}


std::string Sip_Message_Content_RCL::get_string() const
{
    std::string ret = "";
    std::vector <std::string>::const_iterator iter;

    iter = _participants.begin();
    while(iter != _participants.end())\
    {
        ret += (*iter);

        iter++;
        if(iter != _participants.end())
        {
            ret += ",";
        }
    }
    return ret;
}

std::string Sip_Message_Content_RCL::get_content_type() const
{
    return _content_type;
}

std::vector<std::string> Sip_Message_Content_RCL::get_participant_list()
{
    return _participants;
}
