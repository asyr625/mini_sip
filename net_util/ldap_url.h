#ifndef LDAP_URL_H
#define LDAP_URL_H
#include <string>
#include <vector>

#include "my_types.h"

class Ldap_Url_Extension
{
public:
    Ldap_Url_Extension(std::string _type, std::string _value, bool _critical)
        : type(_type), value(_value), critical(_critical)
    {}

    std::string type;
    std::string value;
    bool critical;
};


class Ldap_Url
{
public:
    Ldap_Url(std::string url);
    Ldap_Url();

    void clear();
    bool is_valid() const;

    void set_url(const std::string url);
    std::string get_string() const;

    void print_debug();

    std::string get_host() const;
    void set_host(std::string host_);

    int32_t get_port() const;
    void set_port(int32_t p);

    std::vector<std::string> get_attributes() const;
    void set_attributes(std::vector<std::string> attr);

    std::vector<Ldap_Url_Extension> get_extensions() const;

    bool has_critical_extension() const;

    std::string get_filter() const;
    void set_filter(std::string);

    std::string get_dn() const;
    void set_dn(std::string dn_);

    int32_t get_scope() const;
    void set_scope(int32_t s);
private:
    bool is_unreserved_char(char in) const;
    bool is_reserved_char(char in) const;

    std::string encode_char(const char in) const;
    char decode_char(const std::string in) const;
    int32_t char_to_num(const char in) const;
    std::string percent_decode(const std::string & in) const;
    std::string percent_encode(const std::string & in, bool escapeComma, bool escapeQuestionmark = true) const;

    std::string			host;
    int32_t 			port;
    std::vector<std::string> 	attributes;
    std::vector<Ldap_Url_Extension> 	extensions;
    std::string 			filter;
    std::string 			dn;
    int32_t				scope;

    bool valid_url;
};

#endif // LDAP_URL_H
