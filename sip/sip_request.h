#ifndef SIP_REQUEST_H
#define SIP_REQUEST_H

#include "sip_message.h"
#include "sip_header_route.h"

class Sip_Stack;
class Sip_Response;

class Sip_Request : public Sip_Message
{
public:
    static SRef<Sip_Request*> create_sip_message_ack(
            SRef<Sip_Request*> origReq,
            SRef<Sip_Response*> resp,
            bool provisional=false);

    static SRef<Sip_Request*> create_sip_message_cancel(SRef<Sip_Request*> r );

    static SRef<Sip_Request*> create_sip_message_im_message(
            const std::string &call_id,
            const Sip_Uri& to_uri,
            const Sip_Uri& from_uri,
            int seq_no,
            const std::string &msg);


    static SRef<Sip_Request*> create_sip_message_invite(
            const std::string &call_id,
            const Sip_Uri &to_uri,
            const Sip_Uri &from_uri,
            const Sip_Uri &contact,
            int seq_no,
            SRef<Sip_Stack*> stack
            );

    static SRef<Sip_Request*> create_sip_message_notify(
            const std::string &call_id,
            const Sip_Uri& to_uri,
            const Sip_Uri& from_uri,
            int seq_no
            );

    static SRef<Sip_Request*> create_sip_message_register(
            const std::string &call_id,
            const Sip_Uri &registrar,
            const Sip_Uri &from_uri,
            SRef<Sip_Header_Value_Contact *> contactHdr,
            int seq_no);

    static SRef<Sip_Request*> create_sip_message_subscribe(
            const std::string &call_id,
            const Sip_Uri& to_uri,
            const Sip_Uri& from_uri,
            const Sip_Uri& contact,
            int seq_no);

    static SRef<Sip_Request*> create_sip_message_info(
            const std::string &call_id,
            const Sip_Uri& to_uri,
            const Sip_Uri& toTarget,
            const Sip_Uri& from_uri,
            int seq_no);

    static SRef<Sip_Request*> create_sip_message_info(
            const std::string &call_id,
            const Sip_Uri& to_uri,
            const Sip_Uri& from_uri,
            int seq_no);

    Sip_Request(std::string &build_from);

    Sip_Request(const std::string &method, const Sip_Uri &uri);

    virtual ~Sip_Request();

    virtual std::string get_mem_object_type() const {return "Sip_Request("+_method+")";}

    virtual std::string get_string() const;

    virtual const std::string& get_type();

    virtual void set_method(const std::string &method);
    virtual std::string get_method() const;

    virtual void set_uri(const Sip_Uri &uri);
    virtual const Sip_Uri& get_uri() const;

    void add_route(const std::string &route);

    void add_route(const std::string &addr, int port,
              const std::string &transport);

    void add_routes(const std::list<Sip_Uri> &routes);

    SRef<Sip_Header_Value_Route*> get_first_route();

    void remove_first_route();

    void add_default_headers(const Sip_Uri& from_uri, const Sip_Uri& to_uri, int seq_no, const std::string& call_id="");

protected:
    virtual void init(std::string &build_from);
private:
    std::string _method;
    Sip_Uri _uri;
};

#endif // SIP_REQUEST_H
