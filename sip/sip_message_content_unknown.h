#ifndef SIP_MESSAGE_CONTENT_UNKNOWN_H
#define SIP_MESSAGE_CONTENT_UNKNOWN_H

#include "sip_message_content.h"

SRef<Sip_Message_Content*> sip_unknown_message_content_factory(const std::string &, const std::string &ContentType);

class Sip_Message_Content_Unknown : public Sip_Message_Content
{
public:
    Sip_Message_Content_Unknown(std::string m, std::string contentType);

    virtual std::string get_mem_object_type() const {return "SipMessageContentUnknown";}

    virtual std::string get_string() const;

    virtual std::string get_content_type() const;
private:
    std::string _msg;
    std::string _content_type;
};

#endif // SIP_MESSAGE_CONTENT_UNKNOWN_H
