#include <iostream>

#include "sip_request.h"
#include "sip_exception.h"
#include "sip_uri.h"
#include "sip_stack.h"
#include "sip_header.h"
#include "sip_header_user_agent.h"
#include "sip_header_supported.h"
#include "sip_header_contact.h"
#include "sip_header_event.h"
#include "sip_header_accept.h"
#include "sip_message_content_im.h"

#include "sip_header_from.h"
#include "sip_header_to.h"
#include "sip_header_call_id.h"
#include "sip_header_cseq.h"
#include "sip_header_max_forwards.h"

#include "sip_response.h"

SRef<Sip_Request*> Sip_Request::create_sip_message_ack( SRef<Sip_Request*> origReq, SRef<Sip_Response*> resp, bool provisional)
{
    std::string method;
    if (provisional)
        method = "PRACK";
    else
        method = "ACK";

    SRef<Sip_Request*> req = new Sip_Request(method, origReq->get_uri() );

    req->add_header(new Sip_Header(new Sip_Header_Value_Max_Forwards(70)));

    int noHeaders = origReq->get_no_headers();
    for (int32_t i=0; i< noHeaders; i++)
    {			//FIX: deep copy?
        SRef<Sip_Header *> header = origReq->get_header_no(i);
        int headerType = header->get_type();
        switch (headerType)
        {
            case SIP_HEADER_TYPE_CSEQ:
                ((Sip_Header_Value_CSeq*) *(header->get_header_value(0)))->set_method(method);
            case SIP_HEADER_TYPE_FROM:
            case SIP_HEADER_TYPE_CALLID:
            case SIP_HEADER_TYPE_ROUTE:
            case SIP_HEADER_TYPE_AUTHORIZATION:
            case SIP_HEADER_TYPE_PROXYAUTHORIZATION:
                req->add_header(header);
                break;
            default:
                break;
        }
    }
    req->add_header( new Sip_Header( *resp->get_header_value_to() ) );

    return req;
}

SRef<Sip_Request*> Sip_Request::create_sip_message_cancel( SRef<Sip_Request*> r )
{
    SRef<Sip_Request*> req = new Sip_Request("CANCEL", r->get_uri());

    req->add_header(new Sip_Header( new Sip_Header_Value_Max_Forwards(70)));

    SRef<Sip_Header *> header;
    int noHeaders = r->get_no_headers();
    for (int32_t i=0; i< noHeaders; i++)
    {
        header = r->get_header_no(i);
        int type = header->get_type();
        bool add = false;
        switch (type)
        {
            case SIP_HEADER_TYPE_FROM:
                add = true;
                break;
            case SIP_HEADER_TYPE_TO:
                add = true;
                break;
            case SIP_HEADER_TYPE_CSEQ:
                ((Sip_Header_Value_CSeq*)*(header->get_header_value(0)))->set_cseq(  ((Sip_Header_Value_CSeq *)*(header->get_header_value(0)))->get_cseq() );
                ((Sip_Header_Value_CSeq*)*(header->get_header_value(0)))->set_method("CANCEL");
                add = true;
                break;
            case SIP_HEADER_TYPE_CALLID:
                add = true;
                break;

            // CANCEL requests must have the same branch
            // parameter as the transaction it cancels.
            // If CANCEL packets are treated as any other
            // request, then they would be assigned a random
            // branch. When we create the CANCEL request (how)
            // we therefore copy the Via header to indicate
            // which transaction it cancels. The
            // transport layer must make sure to not add it again.
            case SIP_HEADER_TYPE_VIA:
            case SIP_HEADER_TYPE_AUTHORIZATION:
            case SIP_HEADER_TYPE_PROXYAUTHORIZATION:
                add = true;
                break;
            default:
                break;
        }
        if (add)
            req->add_header(header);
    }
    return req;
}

SRef<Sip_Request*> Sip_Request::create_sip_message_im_message( const std::string &call_id, const Sip_Uri& to_uri,
                                                  const Sip_Uri& from_uri, int seq_no, const std::string &msg)
{
    SRef<Sip_Request*> req = new Sip_Request("MESSAGE", to_uri);
    req->add_default_headers(from_uri, to_uri, seq_no, call_id);
    req->add_header(new Sip_Header(new Sip_Header_Value_User_Agent(HEADER_USER_AGENT_DEFAULT)));
    req->set_content(new Sip_Message_Content_IM(msg));
    return req;
}

static void add_headers( SRef<Sip_Request*> req, const Sip_Uri &contact, SRef<Sip_Stack*> stack )
{

    req->add_header(new Sip_Header(new Sip_Header_Value_Contact(contact)));
    req->add_header(new Sip_Header(new Sip_Header_Value_User_Agent(HEADER_USER_AGENT_DEFAULT)));
    if (stack)
    {
        req->add_header(new Sip_Header(new Sip_Header_Value_Supported(stack->get_all_supported_extensions_str())));
    }
}

SRef<Sip_Request*> Sip_Request::create_sip_message_invite( const std::string &call_id, const Sip_Uri &to_uri, const Sip_Uri &from_uri,
                                              const Sip_Uri &contact, int seq_no, SRef<Sip_Stack*> stack )
{
    SRef<Sip_Request*> req = new Sip_Request("INVITE", to_uri);

    req->add_default_headers( from_uri, to_uri, seq_no, call_id );
    add_headers(req, contact, stack);

    return req;
}

SRef<Sip_Request*> Sip_Request::create_sip_message_notify( const std::string &call_id, const Sip_Uri& to_uri,
                                              const Sip_Uri& from_uri, int seq_no )
{
    SRef<Sip_Request*> req = new Sip_Request("NOTIFY", to_uri);
    req->add_default_headers(from_uri, to_uri, seq_no,call_id);
    req->add_header(new Sip_Header(new Sip_Header_Value_User_Agent(HEADER_USER_AGENT_DEFAULT)));
    req->add_header(new Sip_Header(new Sip_Header_Value_Event("presence")));
    return req;
}

SRef<Sip_Request*> Sip_Request::create_sip_message_register( const std::string &call_id, const Sip_Uri &registrar,
                                                const Sip_Uri &from_uri,  SRef<Sip_Header_Value_Contact *> contactHdr, int seq_no)
{
    SRef<Sip_Request*> req = new Sip_Request("REGISTER", registrar);

    req->add_default_headers(from_uri, from_uri, seq_no, call_id);

    req->add_header(new Sip_Header(*contactHdr));
    req->add_header(new Sip_Header(new Sip_Header_Value_User_Agent(HEADER_USER_AGENT_DEFAULT)));
    req->set_content(NULL);

    return req;
}

SRef<Sip_Request*> Sip_Request::create_sip_message_subscribe( const std::string &call_id, const Sip_Uri& to_uri,
                                                 const Sip_Uri& from_uri, const Sip_Uri& contact, int seq_no)
{
    SRef<Sip_Request*> req = new Sip_Request("SUBSCRIBE", to_uri );

    req->add_default_headers(from_uri, to_uri, seq_no, call_id);
    req->add_header(new Sip_Header(new Sip_Header_Value_Contact(contact)));
    req->add_header(new Sip_Header(new Sip_Header_Value_Event("presence")));
    req->add_header(new Sip_Header(new Sip_Header_Value_Accept("application/xpidf+xml")));

    return req;
}

SRef<Sip_Request*> Sip_Request::create_sip_message_info( const std::string &call_id, const Sip_Uri& to_uri,
                                            const Sip_Uri& toTarget, const Sip_Uri& from_uri, int seq_no)
{
    SRef<Sip_Request*> req = new Sip_Request("INFO", toTarget);
    req->add_default_headers(from_uri, to_uri, seq_no, call_id);
    return req;
}

SRef<Sip_Request*> Sip_Request::create_sip_message_info( const std::string &call_id, const Sip_Uri& to_uri,
                                            const Sip_Uri& from_uri, int seq_no)
{
    return create_sip_message_info(call_id, to_uri, to_uri, from_uri, seq_no);
}


void Sip_Request::add_default_headers(const Sip_Uri& from_uri, const Sip_Uri& to_uri,
                                      int seq_no, const std::string& call_id)
{
    add_header(new Sip_Header(new Sip_Header_Value_From(from_uri)));
    add_header(new Sip_Header(new Sip_Header_Value_To(to_uri)));
    add_header(new Sip_Header(new Sip_Header_Value_Call_ID(call_id)));
    add_header(new Sip_Header(new Sip_Header_Value_CSeq(_method, seq_no)));
    add_header(new Sip_Header(new Sip_Header_Value_Max_Forwards(70)));
}

Sip_Request::Sip_Request(std::string &build_from)
    : Sip_Message(build_from)
{
    init(build_from);
}

Sip_Request::Sip_Request(const std::string &method, const Sip_Uri &uri)
    : _method(method),
      _uri(uri)
{
}

void Sip_Request::init(std::string &build_from)
{
    size_t start = 0;
    size_t pos;
    size_t pos2;
    size_t end = 0;
    //int length = build_from.length();
    std::string requestLine;

    // Skip white space
    start = build_from.find_first_not_of( " \r\n\t", start );
    if( (int)start == (int)std::string::npos ){
        throw Sip_Exception_Invalid_Message("Sip_Request malformed - first line did not contain any non whitespace character");
    }

    end = build_from.find_first_of( "\r\n", start );
    if( (int)end == (int)std::string::npos )
    {
        throw Sip_Exception_Invalid_Message("Sip_Request malformed - only one line");
    }

    requestLine = build_from.substr( start, end - start );
    start = 0;
    end = requestLine.length();

    // Parse method
    pos = requestLine.find( ' ', start );
    if( (int)pos == (int)std::string::npos )
    {
        throw Sip_Exception_Invalid_Message("Sip_Request malformed - could not find method");
    }

    _method = requestLine.substr( start, pos - start );

    // Parse version
    pos2 = requestLine.rfind( ' ', end - 1 );
    if( (int)pos2 == (int)std::string::npos )
    {
        throw Sip_Exception_Invalid_Message("Sip_Request malformed - request line did not contain space between method and version");
    }

    std::string version = requestLine.substr( pos2 + 1, end - pos2 );

    if( version != "SIP/2.0" )
    {
        throw Sip_Exception_Invalid_Message("Sip_Request malformed - unknown version");
    }

    pos2 = requestLine.find_last_not_of( ' ', pos2 );

    _uri = requestLine.substr( pos + 1, pos2 - pos );
    if( !_uri.is_valid() )
    {
        throw Sip_Exception_Invalid_Message("Sip_Request malformed - uri");
    }
}

Sip_Request::~Sip_Request()
{
}


std::string Sip_Request::get_string() const
{
    return get_method() + " " + get_uri().get_request_uri_string() + " SIP/2.0\r\n"
            + get_headers_and_content();
}

const std::string& Sip_Request::get_type()
{
    return _method;
}
void Sip_Request::set_method(const std::string &method)
{
    this->_method = method;
}

std::string Sip_Request::get_method() const
{
    return _method;
}

void Sip_Request::set_uri(const Sip_Uri &uri)
{
    this->_uri = uri;
}

const Sip_Uri& Sip_Request::get_uri() const
{
    return _uri;
}

void Sip_Request::add_route(const std::string &route)
{
    SRef<Sip_Header_Value*> route_value = (Sip_Header_Value*)new Sip_Header_Value_Route( route );
    SRef<Sip_Header*> route_hdr = new Sip_Header( route_value );

    add_before(route_hdr);
}

void Sip_Request::add_route(const std::string &addr, int32_t port,
                            const std::string &transport)
{
    std::string u = "<sip:" + addr;

    if( port )
    {
        char buf[20];
        sprintf(buf, "%d", port);
        u = u + ":" + buf;
    }

    if( transport != "" )
    {
        u = u + ";transport=" + transport;
    }

    u = u + ";lr>";

    add_route( u );
}

void Sip_Request::add_routes(const std::list<Sip_Uri> &routes)
{
    std::list<Sip_Uri>::const_reverse_iterator i;
    std::list<Sip_Uri>::const_reverse_iterator first = routes.rend();

    for( i = routes.rbegin(); i != first; ++i )
    {
        const Sip_Uri &route = *i;
        add_route( route.get_string() );
    }
}

SRef<Sip_Header_Value_Route*> Sip_Request::get_first_route()
{
    SRef<Sip_Header_Value_Route*> route;
    SRef<Sip_Header*> header = get_header_of_type(SIP_HEADER_TYPE_ROUTE, 0);

    if( header )
    {
        route = SRef<Sip_Header_Value_Route*>((Sip_Header_Value_Route*)*(header->get_header_value(0)));
    }
    return route;
}

void Sip_Request::remove_first_route()
{
    SRef<Sip_Header*> hdr = get_header_of_type(SIP_HEADER_TYPE_ROUTE, 0);

    if( hdr->get_no_values() > 1 ){
        hdr->remove_header_value( 0 );
    }
    else{
        remove_header( hdr );
    }
}
