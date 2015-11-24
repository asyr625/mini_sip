#ifndef PRESENCE_MESSAGE_CONTENT_H
#define PRESENCE_MESSAGE_CONTENT_H

#include "sip_message_content.h"
#include "sip_message_content_factory.h"

SRef<Sip_Message_Content*> presence_sip_message_content_factory(const std::string &buf, const std::string &ContentType);

class Presence_Message_Content : public Sip_Message_Content
{
public:
    Presence_Message_Content(std::string from, std::string to, std::string onlineStatus, std::string onlineStatusDesc);
    Presence_Message_Content(const std::string &buildFrom);
    virtual std::string get_mem_object_type() const {return "PresenceMessageContent";}
    virtual std::string get_string() const;
    virtual std::string get_content_type() const{return "application/xpidf+xml";}

private:
    std::string from_uri;
    std::string to_uri;
    std::string document;
    std::string online_status;
    std::string online_status_desc;
    std::string presentity;
};

#endif // PRESENCE_MESSAGE_CONTENT_H
