#include "sip_header_accept_contact.h"

SRef<Sip_Header_Value *> acceptContactFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Accept_Contact(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_accept_contact_factory = acceptContactFactory;

const std::string sipHeaderValueValueTypeStr = "Accept-Contact";

Sip_Header_Value_Accept_Contact::Sip_Header_Value_Accept_Contact(std::string build_from)
    : Sip_Header_Value(SIP_HEADER_TYPE_ACCEPTCONTACT, sipHeaderValueValueTypeStr)
{
    //Initialisation
    _feature_tag = "NOT DEFINED";
    _set_require = false;
    _set_explicit = false;
    unsigned i = 0;
    //cerr<<"SipHeaderValueAcceptContact"+build_from<<endl;
    while ( build_from[i]!=';' && i<build_from.length() )
        i++;

    //Parse featuretag
    i++;
    std::string value="";
    while ( build_from[i]!=';' && i<build_from.length() )
    {
        value += build_from[i];
        i++;
    }
    _feature_tag = value;

    //Parse request, explicit
    i++;
    value="";
    while ( build_from[i]!=';' && i<build_from.length() )
    {
        value += build_from[i];
        i++;
    }

    if(value == "request")
        _set_require = true;
    else if (value == "explicit")
        _set_explicit = true;

    value="";
    while ( build_from[i]!=';' && i<build_from.length() ) {
        i++;
        value += build_from[i];
    }

    if(value == "require")
        _set_require = true;
    else if (value == "explicit")
        _set_explicit = true;
}

Sip_Header_Value_Accept_Contact::Sip_Header_Value_Accept_Contact(std::string ft, bool sr, bool se)
    :Sip_Header_Value(SIP_HEADER_TYPE_ACCEPTCONTACT, sipHeaderValueValueTypeStr)
{
    this->_feature_tag = ft;
    this->_set_require = sr;
    this->_set_explicit = se;
}

Sip_Header_Value_Accept_Contact::~Sip_Header_Value_Accept_Contact()
{
}

std::string Sip_Header_Value_Accept_Contact::get_string() const
{
    std::string answer;
    answer = /*"Accept-Contact: */ "*;" + _feature_tag;

    if(_set_require)
        answer = answer + ";require";

    if(_set_explicit)
        answer = answer + ";explicit";

    return answer;
}
