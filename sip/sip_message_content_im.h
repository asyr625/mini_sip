#ifndef SIP_MESSAGE_CONTENT_IM_H
#define SIP_MESSAGE_CONTENT_IM_H

#include "sip_message_content.h"

SRef<Sip_Message_Content*> sip_im_message_content_factory(const std::string &, const std::string &ContentType);

class Sip_Message_Content_IM : public Sip_Message_Content
{
public:
    Sip_Message_Content_IM(std::string m);

    virtual std::string get_mem_object_type() const {return "SipMessageContentIM";}

    virtual std::string get_string() const;

    virtual std::string get_content_type() const;
private:
    std::string _msg;
};

#endif // SIP_MESSAGE_CONTENT_IM_H
