
#include <iostream>

#include "sip_header_contact.h"
#include "string_utils.h"
#include "sip_dialog_config.h"

SRef<Sip_Header_Value *> contactFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Contact(build_from);
}

Sip_Header_Factory_Func_Ptr sipHeaderContactFactory = contactFactory;

const std::string sipHeaderValueContactTypeStr = "Contact";

Sip_Header_Value_Contact::Sip_Header_Value_Contact(const std::string &build_from)
    : Sip_Header_Value(SIP_HEADER_TYPE_CONTACT,sipHeaderValueContactTypeStr),
      _uri(build_from)
{
    if( !_uri.is_valid() )
    {
        std::cerr << "Sip_Header_Value_Contact::constructor - invalid contact uri '" << build_from << "'" << std::endl;
    }

    _feature_tag = "";
    std::string tmp;
    tmp = get_parameter("expires");
    if( tmp == "" )
    {
        set_expires(DEFAULT_SIPPROXY_EXPIRES_VALUE_SECONDS);
    }
#ifdef DEBUG_OUTPUT
// 	cerr << "Sip_Header_Value_Contact::get_string: _uri="<< _uri.get_string() <<endl;
#endif
}

Sip_Header_Value_Contact::Sip_Header_Value_Contact(const Sip_Uri &contactUri, int expires)
    : Sip_Header_Value(SIP_HEADER_TYPE_CONTACT,sipHeaderValueContactTypeStr)
{
    _uri = contactUri;

    if( expires != -1 )
        this->set_expires(expires);
    else this->remove_parameter("expires" );
}

Sip_Header_Value_Contact::~Sip_Header_Value_Contact()
{
}

std::string Sip_Header_Value_Contact::get_string() const
{
    std::string user = _uri.get_string();

#if 0
    std::string name;
    if (user.find("@")!=string::npos)
    {
        name = "";
        unsigned i=0;
        while (user[i]!='@')
            name=name + user[i++];
        while (user[i]!=':' && user[i]!=';')
            i++;
        string args="";
        while (user[i]!='\0' && i<user.length())
            args=args+user[i++];

        user = name+"@"+uri.getIp()+args;
    }
#endif
    std::string ret = /*"Contact: */ user;
    if( _feature_tag != "" )
        ret += ";" + _feature_tag;
    return user;
}

const Sip_Uri &Sip_Header_Value_Contact::get_uri() const
{
    return _uri;
}

void Sip_Header_Value_Contact::set_uri(const Sip_Uri &uri)
{
    _uri = uri;
}

int Sip_Header_Value_Contact::get_expires() const
{
    return atoi( get_parameter("expires").c_str() );
}
void Sip_Header_Value_Contact::set_expires(int expires)
{
    if( !(expires >= 0 && expires < 100000) )
        expires = DEFAULT_SIPPROXY_EXPIRES_VALUE_SECONDS;
    this->set_parameter( "expires", itoa(expires) );
}
