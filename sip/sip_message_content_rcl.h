#ifndef SIP_MESSAGE_CONTENT_RCL_H
#define SIP_MESSAGE_CONTENT_RCL_H
#include <vector>
#include "sip_message_content.h"


SRef<Sip_Message_Content*> sip_rcl_content_factory(const std::string & buf, const std::string & ContentType);

class Sip_Message_Content_RCL : public Sip_Message_Content
{
public:
    Sip_Message_Content_RCL(std::string t);
    Sip_Message_Content_RCL(std::string content, std::string t);

    virtual std::string get_string() const;
    virtual std::string get_content_type() const;

    virtual std::string get_mem_object_type() const {return "SipMessageContentRCL";}

    std::vector<std::string> get_participant_list();
private:
    std::string _content_type;
    std::vector<std::string> _participants;
};

#endif // SIP_MESSAGE_CONTENT_RCL_H
