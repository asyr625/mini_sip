#ifndef SIP_RESPONSE_H
#define SIP_RESPONSE_H

#include "sip_message.h"
#include "sip_request.h"

class Sip_Response : public Sip_Message
{
public:
    static const std::string type;

    Sip_Response(int status, std::string status_desc, SRef<Sip_Request*> req);
    Sip_Response(std::string &resp);

    virtual std::string get_mem_object_type() const { return "SipResponse"; }

    int get_status_code();

    std::string get_status_desc();

    std::string get_string() const;

    virtual const std::string& get_type() { return type; }

    void add_unsupported(const std::list<std::string> &unsupported);
    SRef<Sip_Request*> get_request() const;
private:
    int _status_code;
    std::string _status_desc;

    std::string _tag;
    SRef<Sip_Request*> _request;
};

#endif // SIP_RESPONSE_H
