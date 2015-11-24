#ifndef SIP_MESSAGE_CONTENT_FACTORY_H
#define SIP_MESSAGE_CONTENT_FACTORY_H

#include <map>
#include "sobject.h"
#include "sip_message_content.h"

typedef SRef<Sip_Message_Content*>(*Sip_Message_Content_Factory_Func_Ptr)(const std::string & buf, const std::string & ContentType);

class SMCF_Collection
{
public:
    void add_factory(std::string content_type, Sip_Message_Content_Factory_Func_Ptr f);
    Sip_Message_Content_Factory_Func_Ptr get_factory(const std::string content_type);
private:
    std::map<std::string, Sip_Message_Content_Factory_Func_Ptr > _factories;
};

#endif // SIP_MESSAGE_CONTENT_FACTORY_H
