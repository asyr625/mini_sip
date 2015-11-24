#ifndef SIP_MESSAGE_CONTENT_MIME_H
#define SIP_MESSAGE_CONTENT_MIME_H

#include <list>
#include "sip_message_content.h"

SRef<Sip_Message_Content*> sip_mime_content_factory(const std::string & buf, const std::string & ContentType);

class Sip_Message_Content_Mime : public Sip_Message_Content
{
public:
    Sip_Message_Content_Mime(std::string t);
    Sip_Message_Content_Mime(std::string t, std::string m, std::string b);
    Sip_Message_Content_Mime(std::string content, std::string t);

    virtual std::string get_string() const;
    virtual std::string get_content_type() const;

    std::string get_content_type_without_parameters() const;
    virtual std::string get_mem_object_type() const {return "SipMessageContentMime";}

    void add_part(SRef<Sip_Message_Content*> part);
    int replace_part(SRef<Sip_Message_Content*> part);

    SRef<Sip_Message_Content*> pop_first_part();
    SRef<Sip_Message_Content*> get_part_by_type(std::string objectType);

    int remove_part_by_type(std::string objectType);
    void set_boundry(std::string boundry);
    std::string get_boundry();
private:
    std::string _message;
    std::string _content_type;
    std::string _boundry;
    std::string _unique_boundry;
    std::list<SRef<Sip_Message_Content*> > _parts;
};

#endif // SIP_MESSAGE_CONTENT_MIME_H
