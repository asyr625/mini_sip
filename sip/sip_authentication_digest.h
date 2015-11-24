#ifndef SIP_AUTHENTICATION_DIGEST_H
#define SIP_AUTHENTICATION_DIGEST_H

#include "sobject.h"
#include "sip_header_authorization.h"
class Sip_Credential;
class Sip_Header_Value_Proxy_Authenticate;

class Sip_Request;

class Sip_Authentication_Digest : public SObject
{
public:
    Sip_Authentication_Digest( SRef<Sip_Header_Value_Proxy_Authenticate*> auth );

    bool update( SRef<Sip_Header_Value_Proxy_Authenticate*> auth );

    SRef<Sip_Header_Value_Authorization*> create_authorization( SRef<Sip_Request*> req) const;

    const std::string &get_realm() const;
    bool get_stale() const;

    int get_type() const { return _type; }

    void set_credential( SRef<Sip_Credential*> credential );

    SRef<Sip_Credential*> get_credential() const;

    static std::string md5_to_string(unsigned char *md5);

private:
    std::string calc_response( SRef<Sip_Request*> req ) const;
    const std::string &get_username() const;
    const std::string &get_password() const;

    int _type;
    std::string _realm;
//    std::string _domain;
    std::string _nonce;
    std::string _opaque;
    bool _stale;
    std::string _algorithm;
    std::string _qop;

    SRef<Sip_Credential*> _cred;

    static std::string _null_str;
};

#endif // SIP_AUTHENTICATION_DIGEST_H
