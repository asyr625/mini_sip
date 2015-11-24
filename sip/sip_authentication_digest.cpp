#include "sip_authentication_digest.h"

#include "sip_dialog_config.h"
#include "sip_request.h"

#include "sip_header_authorization.h"
#include "sip_header_proxy_authenticate.h"
#include "sip_header_proxy_authorization.h"

#include "vmd5.h"
#include "string_utils.h"

#include <iostream>
using namespace std;

std::string Sip_Authentication_Digest::_null_str = string("\0", 1);

Sip_Authentication_Digest::Sip_Authentication_Digest( SRef<Sip_Header_Value_Proxy_Authenticate*> auth )
    : _type(-1), _realm(_null_str), _nonce(_null_str), _opaque(_null_str), _qop(_null_str)
{
    update( auth );
}

bool Sip_Authentication_Digest::update( SRef<Sip_Header_Value_Proxy_Authenticate*> auth )
{
    if( _type > -1 && _type != auth->get_type() && auth->get_type() != SIP_HEADER_TYPE_AUTHENTICATIONINFO )
    {
        my_dbg("signaling/sip") << "SipAuthenticationDigest::update non-matching header type" << endl;
        return false;
    }
    if(auth->get_type() != SIP_HEADER_TYPE_AUTHENTICATIONINFO)
        _type = auth->get_type();

    string realmParam = unquote( auth->get_parameter("realm") );
    if( _realm != _null_str && _realm != realmParam )
    {
        my_dbg("signaling/sip") << "SipAuthenticationDigest::update non-matching realm" << endl;
        return false;
    }
    _realm = realmParam;

    _nonce = unquote( auth->get_parameter("nonce") );
    std::string nextnonce = unquote( auth->get_parameter("nextnonce") );
    if(!nextnonce.empty())
        _nonce = nextnonce;

    if( auth->has_parameter("opaque") )
        _opaque = unquote( auth->get_parameter("opaque") );
    else
        _opaque = _null_str;

    if( auth->has_parameter("qop") )
        _qop = auth->get_parameter("qop");
    else
        _qop = _null_str;

    if( auth->has_parameter("stale") )
    {
        _stale = ( auth->get_parameter("stale") != "false" );
    }
    else
        _stale = false;

    return true;
}

SRef<Sip_Header_Value_Authorization*> Sip_Authentication_Digest::create_authorization( SRef<Sip_Request*> req) const
{
    SRef<Sip_Header_Value_Authorization*> authorization;

    Sip_Uri uri( req->get_uri() );
    string response = calc_response( req );

    if( _type == SIP_HEADER_TYPE_WWWAUTHENTICATE || _type == SIP_HEADER_TYPE_AUTHENTICATIONINFO )
    {
        authorization = new Sip_Header_Value_Authorization(
            get_username(),
            _realm,
            _nonce,
            _opaque == _null_str ? "" : _opaque,
            uri,
            response,
            "Digest"
            );
    }
    else {
        authorization = new Sip_Header_Value_Proxy_Authorization(
            get_username(),
            _realm,
            _nonce,
            _opaque == _null_str ? "" : _opaque,
            uri,
            response,
            "Digest"
            );
    }

    return authorization;
}

const std::string &Sip_Authentication_Digest::get_realm() const
{
    return _realm;
}

bool Sip_Authentication_Digest::get_stale() const
{
    return _stale;
}

std::string Sip_Authentication_Digest::md5_to_string(unsigned char *md5)
{
    const char *digits = {"0123456789abcdef"};
    char strsum[33] = {'\0'};
    for (int i=0 ; i<16; i++)
    {
        int32_t intval = md5[i];
        strsum[2*i] = digits[(intval & 0xF0) >>4];
        strsum[2*i+1] = digits[(intval & 0x0F)];
    }
    return string(strsum);
}

std::string Sip_Authentication_Digest::calc_response( SRef<Sip_Request*> req ) const
{
    unsigned char digest[16];
    MD5Context context;
    MD5_init(&context);
    string u_r_p(get_username()+":"+_realm+":"+get_password());
    MD5_update(&context, (const unsigned char *)u_r_p.c_str(), (unsigned int)u_r_p.length() );
    MD5_final(digest,&context);
    string md5_u_r_p = md5_to_string(digest);

    string uri_part = req->get_uri().get_request_uri_string();
    MD5Context c2;
    MD5_init(&c2);
    string uristr(req->get_method()+":"+ uri_part);
    //cerr << "DEBUG: uri_part="<< uri_part<< " sip_method = "<<sip_method<< endl;
    MD5_update(&c2, (const unsigned char *)uristr.c_str(), (unsigned int)uristr.length() );
    MD5_final(digest,&c2);
    string md5_uri = md5_to_string(digest);

    MD5Context c3;
    MD5_init(&c3);
    string all(md5_u_r_p+":"+_nonce+":"+md5_uri);
    MD5_update(&c3,(const unsigned char *)all.c_str(), (unsigned int)all.length() );
    MD5_final(digest,&c3);

    string auth_string = md5_to_string(digest);
    return auth_string;
}

void Sip_Authentication_Digest::set_credential( SRef<Sip_Credential*> credential )
{
    _cred = credential;
}

SRef<Sip_Credential*> Sip_Authentication_Digest::get_credential() const
{
    return _cred;
}

const std::string &Sip_Authentication_Digest::get_username() const
{
    static string anonymous = "anonymous";

    if( _cred.is_null() )
        return anonymous;
    else
        return _cred->get_username();
}
const std::string &Sip_Authentication_Digest::get_password() const
{
    static string empty = "";

    if( _cred.is_null() )
        return empty;
    else
        return _cred->get_password();
}
