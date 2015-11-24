#ifndef SIP_HEADER_AUTHORIZATION_H
#define SIP_HEADER_AUTHORIZATION_H

#include "sip_uri.h"
#include "sip_header.h"

extern Sip_Header_Factory_Func_Ptr sip_Header_authorization_factory;

class Sip_Header_Value_Authorization : public Sip_Header_Value
{
public:
    Sip_Header_Value_Authorization();

    Sip_Header_Value_Authorization(int type, const std::string &typeStr);

    Sip_Header_Value_Authorization(const std::string &build_from);
    Sip_Header_Value_Authorization(int type, const std::string &build_from, const std::string &typeStr);

    Sip_Header_Value_Authorization(const std::string &username,
            const std::string &realm,
            const std::string &nonce,
            const std::string &opaque,
            const Sip_Uri &uri,
            const std::string &response,
            const std::string &auth_meth="DIGEST");

    Sip_Header_Value_Authorization(int type,
                                   const std::string &username,
                                   const std::string &realm,
                                   const std::string &nonce,
                                   const std::string &opaque,
                                   const Sip_Uri &uri,
                                   const std::string &response,
                                   const std::string &auth_meth,
                                   const std::string &typeStr);


    virtual ~Sip_Header_Value_Authorization();

    virtual std::string get_mem_object_type() const {return "SipHeaderAuthorization";}

    std::string get_string() const;

    const std::string &get_auth_method() const;
    void set_auth_method(const std::string &n);

    void set_parameter(const std::string &name,
                      const std::string &value);

    std::string get_username() const;
    void set_username(const std::string &un);

    std::string get_realm() const;
    void set_realm(const std::string &r);

    std::string get_nonce() const;
    void set_nonce(const std::string &n);

    std::string get_opaque() const;
    void set_opaque(const std::string &n);

    Sip_Uri get_uri() const;
    void set_uri(const Sip_Uri &uri);

    std::string get_response() const;
    void set_response(const std::string &resp);

protected:
    char get_first_parameter_separator() const {return ' ';}
    char get_parameter_separator() const {return ',';}

private:
    void init(const std::string& build_from);

    std::string _auth_method;
};

#endif // SIP_HEADER_AUTHORIZATION_H
