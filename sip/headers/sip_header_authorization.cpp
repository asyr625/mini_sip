#include "sip_header_authorization.h"
#include "string_utils.h"

SRef<Sip_Header_Value *> authorizationFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Authorization(build_from);
}

Sip_Header_Factory_Func_Ptr sip_Header_authorization_factory = authorizationFactory;

const std::string sipHeaderValueAuthorizationTypeString="Authorization";

Sip_Header_Value_Authorization::Sip_Header_Value_Authorization(int type, const std::string &typeStr)
    : Sip_Header_Value(type, typeStr)
{
}

Sip_Header_Value_Authorization::Sip_Header_Value_Authorization(const std::string &build_from)
    : Sip_Header_Value(SIP_HEADER_TYPE_AUTHORIZATION, sipHeaderValueAuthorizationTypeString)
{
    init( build_from );
}

Sip_Header_Value_Authorization::Sip_Header_Value_Authorization(int type, const std::string &build_from, const std::string &typeStr)
    : Sip_Header_Value(type,typeStr)
{
    init( build_from );
}

void Sip_Header_Value_Authorization::init(const std::string& build_from)
{
    size_t pos = build_from.find_first_not_of(" \t\r\n");
    size_t last = build_from.find_first_of(" \t\r\n", pos);

    if( last != std::string::npos )
    {
        _auth_method = build_from.substr( pos, last - pos );
        add_parameter( new Sip_Header_Parameter( build_from.substr( last )));
    }
}

Sip_Header_Value_Authorization::Sip_Header_Value_Authorization(
        const std::string &username,
        const std::string &realm,
        const std::string &nonce,
        const std::string &opaque,
        const Sip_Uri &uri,
        const std::string &response,
        const std::string &auth_meth)
    : Sip_Header_Value(SIP_HEADER_TYPE_AUTHORIZATION, sipHeaderValueAuthorizationTypeString),
      _auth_method(auth_meth)
{
    set_parameter("algorithm", "MD5");
    set_parameter("username", quote(username));
    set_parameter("realm", quote(realm));
    set_parameter("nonce", quote(nonce));
    set_parameter("uri", quote(uri.get_request_uri_string()));
    set_parameter("response", quote(response));
    if( opaque != "")
        set_parameter("opaque", quote(opaque));
}

Sip_Header_Value_Authorization::Sip_Header_Value_Authorization(int type,
                                                               const std::string &username,
                                                               const std::string &realm,
                                                               const std::string &nonce,
                                                               const std::string &opaque,
                                                               const Sip_Uri &uri,
                                                               const std::string &response,
                                                               const std::string &auth_meth,
                                                               const std::string &typeStr)
    : Sip_Header_Value(type,typeStr),
      _auth_method(auth_meth)
{
    set_parameter("algorithm", "MD5");
    set_parameter("username", quote(username));
    set_parameter("realm", quote(realm));
    set_parameter("nonce", quote(nonce));
    set_parameter("uri", quote(uri.get_request_uri_string()));
    set_parameter("response", quote(response));
    if( opaque != "")
        set_parameter("opaque", quote(opaque));
}


Sip_Header_Value_Authorization::~Sip_Header_Value_Authorization()
{
}

std::string Sip_Header_Value_Authorization::get_string() const
{
    return _auth_method;
}

const std::string &Sip_Header_Value_Authorization::get_auth_method() const
{
    return _auth_method;
}
void Sip_Header_Value_Authorization::set_auth_method(const std::string &n)
{
    _auth_method = n;
}

void Sip_Header_Value_Authorization::set_parameter(const std::string &name, const std::string &value)
{
    add_parameter(new Sip_Header_Parameter(name, value, true));
}

std::string Sip_Header_Value_Authorization::get_username() const
{
    return get_parameter("username");
}
void Sip_Header_Value_Authorization::set_username(const std::string &un)
{
    set_parameter("username", un);
}

std::string Sip_Header_Value_Authorization::get_realm() const
{
    return get_parameter("realm");
}
void Sip_Header_Value_Authorization::set_realm(const std::string &r)
{
    set_parameter("realm", r);
}

std::string Sip_Header_Value_Authorization::get_nonce() const
{
    return get_parameter("none");
}
void Sip_Header_Value_Authorization::set_nonce(const std::string &n)
{
    set_parameter("nonce", n);
}

std::string Sip_Header_Value_Authorization::get_opaque() const
{
    return get_parameter("opaque");
}
void Sip_Header_Value_Authorization::set_opaque(const std::string &n)
{
    set_parameter("opaque", n);
}

Sip_Uri Sip_Header_Value_Authorization::get_uri() const
{
    return get_parameter("uri");
}
void Sip_Header_Value_Authorization::set_uri(const Sip_Uri &uri)
{
    set_parameter("uri", uri.get_request_uri_string());
}
