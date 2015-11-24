#include "sip_response.h"
#include "sip_header_unsupported.h"
#include "sip_header_max_forwards.h"
#include "sip_exception.h"
#include "string_utils.h"

#include <stdlib.h>

const std::string Sip_Response::type = "RESPONSE";

Sip_Response::Sip_Response(int status, std::string status_desc, SRef<Sip_Request*> req)
    : _request(req)
{
    this->_status_code = status;
    this->_status_desc = status_desc;

    set_content(NULL);

    SRef<Sip_Header_Value*> mf = new Sip_Header_Value_Max_Forwards(70);
    add_header(new Sip_Header(*mf));

    int noHeaders = req->get_no_headers();
    for (int i=0 ; i < noHeaders; i++)
    {			//FIX: deep copy
        SRef<Sip_Header*> header = req->get_header_no(i);
        int headerType = header->get_type();
        switch (headerType)
        {
            case SIP_HEADER_TYPE_VIA:
            case SIP_HEADER_TYPE_FROM:
            case SIP_HEADER_TYPE_TO:
            case SIP_HEADER_TYPE_CALLID:
            case SIP_HEADER_TYPE_CSEQ:
            case SIP_HEADER_TYPE_RECORDROUTE: //if it exists in the request, it is copied to response
                add_header(header);
                break;
            default:
                /*don't copy any other header*/
                break;
        }
    }
}

Sip_Response::Sip_Response(std::string &resp)
    : Sip_Message(resp)
{
    size_t len = resp.size();

    size_t i = 0;

    //If stream transport we should allow whitespace before the start
    //of the message
    while (i<len &&(resp[i]==' ' || resp[i]=='\r' || resp[i]=='\n' || resp[i]=='\t'))
        i++;

        //strlen(SIP/2.0)==7
    if (!( i+7<=len && resp.substr(i,7)=="SIP/2.0" ) )
    {
        throw Sip_Exception_Invalid_Message("SipResponse header error");
    }
    i+=7;

    size_t afterws = i;
    while ( afterws<len && resp[afterws]!='\0' && (resp[afterws]==' ' || resp[afterws]=='\t'))
        afterws++;

    if (afterws + 3 >= len)
    {
        throw Sip_Exception_Invalid_Message("SipResponse header error");
    }

    if (! (resp[afterws+0]>='0' && resp[afterws+0]<='9' &&
                resp[afterws+1]>='0' && resp[afterws+1]<='9' &&
                resp[afterws+2]>='0' && resp[afterws+2]<='9' ) )
    {
        throw Sip_Exception_Invalid_Message("SipResponse without status code");
    }

    _status_code = (resp[afterws+0]-'0')*100 + (resp[afterws+1]-'0')*10 + resp[afterws+2]-'0';

    _status_desc = "";
    i = afterws + 3;	//go past response code
    while( resp[i] == ' ' )
      i++;

    for ( ; resp[i]!='\r' && resp[i]!='\n'; i++)
    {
        if(len == i){
#ifdef DEBUG_OUTPUT
        my_dbg("signaling/sip") << "SipResponse::SipResponse: message did not end correctly - throwing exception"<< endl;
#endif

            throw Sip_Exception_Invalid_Message("SipResponse malformed - could not find end of response description");
        }
        _status_desc = _status_desc + resp[i];
    }
}

int Sip_Response::get_status_code()
{
    return _status_code;
}

std::string Sip_Response::get_status_desc()
{
    return _status_desc;
}

std::string Sip_Response::get_string() const
{
    std::string rep = "SIP/2.0 "+ itoa(_status_code) + " " + _status_desc + "\r\n";
    rep = rep + get_headers_and_content();
    return rep;
}

void Sip_Response::add_unsupported(const std::list<std::string> &unsupported)
{
    std::list<std::string>::const_iterator i;
    std::list<std::string>::const_iterator last = unsupported.end();

    for( i = unsupported.begin(); i != last; i++ ){
        const std::string &ext = *i;

        Sip_Header_Value *value = new Sip_Header_Value_Unsupported( ext );
        add_header( new Sip_Header( value ));
    }
}

SRef<Sip_Request*> Sip_Response::get_request() const
{
    return _request;
}
