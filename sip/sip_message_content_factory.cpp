#include "sip_message_content_factory.h"

void SMCF_Collection::add_factory(std::string content_type, Sip_Message_Content_Factory_Func_Ptr f)
{
    _factories[content_type] = f;
}

Sip_Message_Content_Factory_Func_Ptr SMCF_Collection::get_factory(const std::string content_type)
{
    size_t index  = content_type.find("; ",0);
    std::string str;
    if (index != std::string::npos)
    {
        str = content_type.substr(0,index);
    }
    else
        str = content_type;
    return _factories[str];
}
