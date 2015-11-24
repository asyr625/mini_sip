#ifndef SIP_HEADER_ACCEPT_CONTACT_H
#define SIP_HEADER_ACCEPT_CONTACT_H

#include "sip_header.h"

extern Sip_Header_Factory_Func_Ptr sip_header_accept_contact_factory;

class Sip_Header_Value_Accept_Contact : public Sip_Header_Value
{
public:
    Sip_Header_Value_Accept_Contact();
    Sip_Header_Value_Accept_Contact(std::string build_from);
    Sip_Header_Value_Accept_Contact(std::string ft, bool sr, bool se);

    virtual ~Sip_Header_Value_Accept_Contact();

    std::string get_mem_object_type() const { return "SipHeaderAcceptContact"; }

    std::string get_string() const ;

    std::string get_feature_tag() const { return _feature_tag; }

private:
    std::string _feature_tag;
    bool _set_require;
    bool _set_explicit;
};

#endif // SIP_HEADER_ACCEPT_CONTACT_H
