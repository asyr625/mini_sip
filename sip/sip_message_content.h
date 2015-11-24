#ifndef SIP_MESSAGE_CONTENT_H
#define SIP_MESSAGE_CONTENT_H

#include "sobject.h"

class Sip_Message_Content : public SObject
{
public:
    virtual std::string get_string() const = 0;
    virtual std::string get_content_type() const = 0;
};

#endif // SIP_MESSAGE_CONTENT_H
