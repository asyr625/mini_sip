#ifndef SIP_MESSAGE_H
#define SIP_MESSAGE_H

#include <list>
#include "sobject.h"
#include "sip_message_content.h"
#include "mini_list.h"
#include "sip_uri.h"
#include "socket.h"
#include "sip_message_content_factory.h"

#include "sip_header_authentication_info.h"

class Sip_Header;
//class Socket;

class Sip_Header_Value;
class Sip_Header_Value_Contact;
class Sip_Header_Value_Expires;
class Sip_Header_Value_From;
class Sip_Header_Value_To;
class Sip_Header_Value_Via;
class Sip_Header_Value_Proxy_Authenticate;
class Sip_Header_Value_WWW_Authenticate;


SRef<Sip_Message_Content*> sipSipMessageContentFactory(const std::string & buf, const std::string & ContentType);

class Sip_Message : public Sip_Message_Content
{
public:
    static const std::string anyType;

    virtual std::string get_content_type() const { return "message/sipfrag"; }

    static SMCF_Collection content_factories;
protected:
    Sip_Message();
    Sip_Message(std::string &build_from);

public:
    virtual ~Sip_Message();

    static SRef<Sip_Message*> create_message(std::string &data);

    virtual const std::string& get_type() = 0;

    void add_header(SRef<Sip_Header*> header);

    void add_before(SRef<Sip_Header*> h);

    bool requires(std::string extension);

    std::list<std::string> get_required();

    bool supported(std::string extension);

    int get_content_length();

    SRef<Sip_Header_Value_From*> get_header_value_from();

    SRef<Sip_Header_Value_To*> get_header_value_to();

    SRef<Sip_Header_Value_Contact*> get_header_value_contact();

    SRef<Sip_Header_Value_Expires*> get_header_value_expires();

    void set_content(SRef<Sip_Message_Content*> c);

    SRef<Sip_Message_Content *> get_content();

    int get_cseq();

    std::string get_cseq_method();

    SRef<Sip_Header_Value_Via*> get_first_via();

    void remove_first_via();

    std::string get_call_id();

    Sip_Uri get_from();

    Sip_Uri get_to();

    virtual std::string get_string() const =0;

    virtual std::string get_headers_and_content() const;

    std::string get_warning_message();


    friend std::ostream & operator<<(std::ostream &out, Sip_Message &);
    std::string get_description();

    std::string get_branch();

    int get_no_headers();

    SRef<Sip_Header*> get_header_no(int i);

    SRef<Sip_Header_Value*> get_header_value_no(int type, int i);

    std::list<std::string> get_route_set();

    void set_socket(SRef<Socket*> s);

    SRef<Socket*> get_socket();

    std::string get_authenticate_property(std::string prop);

    SRef<Sip_Header_Value_Proxy_Authenticate*> get_header_value_proxy_authenticate(int i);

    SRef<Sip_Header_Value_Authentication_Info *> get_header_value_authentication_info(const int &i);

    SRef<Sip_Header_Value_WWW_Authenticate *> get_header_value_wwwauthenticate(int i);

    void remove_header_value(SRef<Sip_Header_Value*> hval);

protected:

    bool add_line(std::string line);

    void remove_header(SRef<Sip_Header*> header);

    SRef<Sip_Header*> get_header_of_type(int t, int i=0);
private:
    Mini_List<SRef<Sip_Header*> >   _headers;

    SRef<Sip_Message_Content*> _content;

    int parse_headers(const std::string &buf, int startIndex);

    SRef<Socket*> sock;
};

#endif // SIP_MESSAGE_H
