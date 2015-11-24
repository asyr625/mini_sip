#ifndef SIP_HEADER_CONTACT_H
#define SIP_HEADER_CONTACT_H

#include "sip_header.h"
#include "sip_uri.h"


extern Sip_Header_Factory_Func_Ptr sip_header_contact_factory;

class Sip_Header_Value_Contact : public Sip_Header_Value
{
public:
    Sip_Header_Value_Contact(const std::string &build_from);
    Sip_Header_Value_Contact(const Sip_Uri &contactUri, int expires=1000);

    virtual ~Sip_Header_Value_Contact();

    virtual std::string get_mem_object_type() const { return "SipHeaderContact"; }

    std::string get_string() const;

    const Sip_Uri &get_uri() const;
    void set_uri(const Sip_Uri &uri);

    void set_feature_tag(std::string ft) { this->_feature_tag = ft; }

    int get_expires() const;
    void set_expires(int expires);

private:
   Sip_Uri _uri;
   std::string _feature_tag;
};

#endif // SIP_HEADER_CONTACT_H
