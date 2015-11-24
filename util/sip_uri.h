#ifndef SIP_URI_H
#define SIP_URI_H
#include <map>
#include "sobject.h"

class Sip_Uri : public SObject
{
public:
    Sip_Uri() { clear(); }

    ~Sip_Uri();

    Sip_Uri(std::string build_from);
    void set_uri( std::string buildFrom );

    void parse_user_info(std::string UriData );

    void set_params(std::string userName, std::string ip_, std::string type, int port_);

    std::string get_string() const;

    std::string get_user_ip_string() const;

    std::string get_request_uri_string() const;

    virtual std::string get_mem_object_type() const { return "SipUri"; }

    void set_display_name(std::string id);
    const std::string & get_display_name() const;

    void set_protocol_id(std::string protocol_id);
    const std::string & get_protocol_id() const;

    void set_user(std::string id);
    const std::string & get_user_name() const;

    void set_ip(std::string ip);
    const std::string & get_ip() const;

    void set_port(int port);
    int get_port() const;

    void set_user_type(std::string user_type);
    const std::string & get_user_type() const;

    void set_transport(std::string transp);
    const std::string & get_transport() const;

    bool is_valid() const;

    void make_valid( bool valid );

    void clear();

    void set_parameter(const std::string &key, const std::string &val);

    bool has_parameter(const std::string &key) const;

    const std::string & get_parameter(const std::string &key) const;

    void remove_parameter(const std::string &key);

    int operator==( const Sip_Uri &uri ) const;
private:
    std::string _display_name;
    std::string _protocol_id;
    std::string _user_name;
    std::string _ip;
    int _port;

    bool _valid_uri;
    std::map<std::string, std::string> _parameters;
};

std::ostream& operator << (std::ostream& os, const Sip_Uri& uri);

#endif // SIP_URI_H
