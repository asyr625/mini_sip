#include <iostream>
#include "sip_message.h"

#include "sip_message_content.h"
#include "sip_message_content_unknown.h"

#include "sip_header_via.h"
#include "sip_header_from.h"
#include "sip_header_to.h"
#include "sip_header_call_id.h"
#include "sip_header_cseq.h"
#include "sip_header_content_length.h"
#include "sip_header_contact.h"
#include "sip_header_expires.h"
#include "sip_header_record_route.h"
#include "sip_header_route.h"
#include "sip_header_require.h"
#include "sip_header_supported.h"
#include "sip_header_content_type.h"
#include "sip_header_www_authenticate.h"
#include "sip_header_authentication_info.h"
#include "sip_header_warning.h"

#include "sip_utils.h"
#include "sip_request.h"
#include "sip_response.h"
#include "sip_exception.h"
#include "string_utils.h"

#include "dbg.h"


const std::string Sip_Message::anyType = "";

#if defined(_MSC_VER) && !defined(_WIN32_WCE)
template class __declspec(dllexport) SRef<Sip_Message*>;
#endif

SRef<Sip_Message_Content*> sipSipMessageContentFactory(const std::string & buf, const std::string & ContentType)
{
    std::string tmp = buf;
    return (*Sip_Message::create_message(tmp));
}

SMCF_Collection Sip_Message::content_factories = SMCF_Collection();

std::string Sip_Message::get_description()
{
    std::string ret;
    ret = get_type();
    if (ret == "RESPONSE")
        ret += "_" + ((Sip_Response*)(this))->get_status_desc();//itoa
    return ret;
}

Sip_Message::Sip_Message()
{
}

Sip_Message::Sip_Message(std::string &buildFrom)
{
    uint32_t i;

    uint32_t start = 0;
    uint32_t blen = buildFrom.size();

    //skip whitespace (can be there if received over reliable transport)
    while (start<blen && is_ws(buildFrom[start]))
        start++;

    //Skip first line (they are parsed by sub-class)
    for (i = start; buildFrom[i]!='\r' && buildFrom[i]!='\n'; i++)
    {
        if(i == blen)
        {
#ifdef DEBUG_OUTPUT
            std::cerr << "SipMessage::SipMessage: Size is too short - throwing exception"<< std::endl;
#endif
            throw Sip_Exception_Invalid_Message("SIP Message too short");
        }
    }

    int contentStart = parse_headers(buildFrom, i);
    int clen = get_content_length();
    if (clen>0)
    {
        std::string contentbuf=buildFrom.substr(contentStart, clen);
        if ((int)contentbuf.length() != clen)
            std::cerr << "WARNING: Length of content was shorter than expected (" << clen <<"!="<<(int)contentbuf.length()<<")"<<std::endl;

        SRef<Sip_Header*> h = get_header_of_type(SIP_HEADER_TYPE_CONTENTTYPE);
        if (h)
        {
            std::string contentType = ((Sip_Header_Value_String*)*(h->get_header_value(0) ))->get_string();
            //			string b = (Sip_Header_ValueContentType*)*(h->get_header_value(0) )->get_parameter("boundary");
            //std::cerr <<"boundary="<< b <<std::endl;
            Sip_Message_Content_Factory_Func_Ptr contentFactory = content_factories.get_factory( contentType );
            if (contentFactory)
            {
                SRef<Sip_Message_Content*> smcref = contentFactory(contentbuf, contentType );
                set_content( smcref );
            }
            else
                set_content( new Sip_Message_Content_Unknown( contentbuf, contentType ));
        }
        else
        {
            my_err << "WARNING: unknown content type"<<std::endl;
            set_content( new Sip_Message_Content_Unknown( contentbuf, "unknown" ));
        }
    }
}

Sip_Message::~Sip_Message()
{
}

SRef<Sip_Message*> Sip_Message::create_message(std::string &data)
{
    size_t n = data.size();
    size_t start = 0;

    while ( start < n && is_ws(data[start]))
        start++;

    if ( n > 3 && (data[start+0]=='S'||data[start+0]=='s') &&
         (data[start+1]=='I'||data[start+1]=='i') &&
         (data[start+2]=='P'||data[start+2]=='p' ))
    {
        return SRef<Sip_Message*>(new Sip_Response(data));
    }
    else
    {
        return new Sip_Request(data);
    }
    return NULL;
}

std::ostream & operator<<(std::ostream &out, Sip_Message &p)
{
    out << p.get_description();
    return out;
}

void Sip_Message::add_header(SRef<Sip_Header*> header)
{
    if( header.is_null() )
    {
        my_err << "ERROR: trying to add null header to message!"<<std::endl;
        return;
    }
    _headers.push_back(header);
}


SRef<Sip_Header*> Sip_Message::get_header_no(int i)
{
    if (i >= _headers.size())
    {
        SRef<Sip_Header*> nullhdr;
        return nullhdr;
    }
    return _headers[i];
}

int Sip_Message::get_no_headers()
{
    return _headers.size();
}

int Sip_Message::get_content_length()
{
    SRef<Sip_Header_Value*> cl = get_header_value_no( SIP_HEADER_TYPE_CONTENTLENGTH, 0 );
    if (cl)
        return ((Sip_Header_Value_Content_Length*)*cl)->get_content_length();
    else
        return 0;
}

bool Sip_Message::add_line(std::string line)
{
    SRef<Sip_Header*> hdr = Sip_Header::parse_header(line);
    if( hdr.is_null() )
        return false;
    add_header(hdr);
    return true;
}

void Sip_Message::add_before(SRef<Sip_Header*> h)
{
    int htype = h->get_type();
    int i;
    int pos = 0;

    for( i = 0; i < _headers.size(); i++ )
    {
        if( _headers[i]->get_type() == htype)
        {
            pos = i;
            break;
        }
    }

    _headers.insert( pos, h );
}

bool Sip_Message::requires(std::string extension)
{
    SRef<Sip_Header_Value*> hval;
    bool ret = false;
    int i = 0;
    bool done = false;

    do{
        hval = get_header_value_no(SIP_HEADER_TYPE_REQUIRE, i);
        if (hval){
            std::string e= ((Sip_Header_Value_Require*)*hval)->get_string();
            if (e == extension)
            {
                ret = true;
                done = true;
            }
        }
        i++;
    }while(!done && hval);

    return ret;
}

std::list<std::string> Sip_Message::get_required()
{
    std::list<std::string> set;
    int n=0;
    SRef<Sip_Header_Value*> rr;
    while ( ! (rr=get_header_value_no(SIP_HEADER_TYPE_REQUIRE,n++)).is_null() )
    {
        set.push_back( ((Sip_Header_Value_Require*)*rr)->get_string() );
    }

    return set;
}

bool Sip_Message::supported(std::string extension)
{
    SRef<Sip_Header_Value*> hval;
    bool ret = false;
    int i = 0;
    bool done = false;

    do{
        hval = get_header_value_no(SIP_HEADER_TYPE_SUPPORTED, i);
        if (hval)
        {
            std::string e = ((Sip_Header_Value_Supported*)*hval)->get_string();
            if (e == extension)
            {
                ret = true;
                done= true;
            }
        }
        i++;
    }while(!done && hval);

    return ret;
}

SRef<Sip_Header_Value_From*> Sip_Message::get_header_value_from()
{
    SRef<Sip_Header*> from = get_header_of_type( SIP_HEADER_TYPE_FROM );
    if( from )
        return SRef<Sip_Header_Value_From*>( (Sip_Header_Value_From*)*(from->get_header_value(0)) );

    return NULL;
}

SRef<Sip_Header_Value_To*> Sip_Message::get_header_value_to()
{
    SRef<Sip_Header*> to = get_header_of_type( SIP_HEADER_TYPE_TO );

    if( to )
        return SRef<Sip_Header_Value_To*>( (Sip_Header_Value_To*)*(to->get_header_value(0)) );

    return NULL;
}

SRef<Sip_Header_Value_Contact*> Sip_Message::get_header_value_contact()
{
    SRef<Sip_Header *> h = get_header_of_type( SIP_HEADER_TYPE_CONTACT );

    if( h )
        return (Sip_Header_Value_Contact*)*(h->get_header_value(0) );
    return NULL;
}

SRef<Sip_Header_Value_Expires*> Sip_Message::get_header_value_expires()
{
    SRef<Sip_Header *> h = get_header_of_type( SIP_HEADER_TYPE_EXPIRES );

    if( h )
        return (Sip_Header_Value_Expires*)*(h->get_header_value(0) );
    return NULL;
}

void Sip_Message::set_content(SRef<Sip_Message_Content*> c)
{
    this->_content = c;
    if( _content )
    {
        std::string contentType = _content->get_content_type();
        if( contentType != "" )
        {
            SRef<Sip_Header_Value*> hdr = get_header_value_no(SIP_HEADER_TYPE_CONTENTTYPE, 0);

            if( hdr )
            {
                SRef<Sip_Header_Value_Content_Type*> contentTypeHdr = (Sip_Header_Value_Content_Type*)*hdr;
                contentTypeHdr->set_string(contentType);
            }
            else{
                SRef<Sip_Header_Value*> contenttypep = new Sip_Header_Value_Content_Type( contentType );
                add_header( new Sip_Header(contenttypep) );
            }
        }
    }
}

SRef<Sip_Message_Content *> Sip_Message::get_content()
{
    return _content;
}

std::string Sip_Message::get_call_id()
{
    SRef<Sip_Header_Value*> id = get_header_value_no( SIP_HEADER_TYPE_CALLID, 0 );
    if (id)
        return ((Sip_Header_Value*)*id)->get_string();
    else
        return "";
}

int Sip_Message::get_cseq()
{
    SRef<Sip_Header_Value*> seq = get_header_value_no( SIP_HEADER_TYPE_CSEQ, 0 );
    if (seq)
        return ((Sip_Header_Value_CSeq*)*seq)->get_cseq();
    else{
        my_dbg("signaling/sip") << "ERROR: Could not find command sequence number in sip Message."<< std::endl;
        return -1;
    }
}

std::string Sip_Message::get_cseq_method()
{
    SRef<Sip_Header_Value*> seq = get_header_value_no( SIP_HEADER_TYPE_CSEQ, 0 );
    if (seq)
        return ((Sip_Header_Value_CSeq*)*seq)->get_method();
    else{
        my_dbg("signaling/sip") << "ERROR: Could not find command sequence method in sip Message."<< std::endl;
        return "";
    }
}

SRef<Sip_Header_Value_Via*> Sip_Message::get_first_via()
{
    SRef<Sip_Header_Value*> via = get_header_value_no( SIP_HEADER_TYPE_VIA, 0 );
    if (via)
        return (Sip_Header_Value_Via*) *via;
    else
        return NULL;
}

void Sip_Message::remove_first_via()
{
    SRef<Sip_Header*> hdr = get_header_of_type( SIP_HEADER_TYPE_VIA, 0 );
    if( hdr->get_no_values() > 1 )
        hdr->remove_header_value( 0 );
    else
        remove_header( hdr );
}

Sip_Uri Sip_Message::get_from()
{
    Sip_Uri ret;
    SRef<Sip_Header_Value_From*> hfrom = get_header_value_from();
    if (hfrom)
        ret = hfrom->get_uri();
    return ret;
}

Sip_Uri Sip_Message::get_to()
{
    Sip_Uri ret;
    SRef<Sip_Header_Value_To*> hto = get_header_value_to();
    if (hto)
        ret = hto->get_uri();
    return ret;
}

std::string Sip_Message::get_headers_and_content() const
{
    std::string req = "";
    int32_t clen = -1;

    for (int32_t i=0; i< _headers.size(); i++)
    {
        req = req + _headers[i]->get_string() + "\r\n";
        if ((_headers[i])->get_type() == SIP_HEADER_TYPE_CONTENTLENGTH)
            clen = 0;
    }
    if (clen<0)
    {
        if ( !_content.is_null())
            clen=(int)_content->get_string().length();
        else
            clen=0;
        Sip_Header content_length(new Sip_Header_Value_Content_Length(clen));
        req = req + content_length.get_string() + "\r\n";
    }
    req = req + "\r\n";

    if ( !_content.is_null())
        req = req + _content->get_string();
    return req;
}

std::string Sip_Message::get_warning_message()
{
    SRef<Sip_Header_Value*> warn = get_header_value_no( SIP_HEADER_TYPE_WARNING, 0 );
    if (warn)
        return ((Sip_Header_Value_Warning*)*warn)->get_warning();
    else
        return "";
}

std::string Sip_Message::get_branch()
{
    SRef<Sip_Header_Value*> firstVia = get_header_value_no( SIP_HEADER_TYPE_VIA, 0 );
    if (firstVia)
        return firstVia->get_parameter("branch");
    else
        return "";
}


SRef<Sip_Header_Value*> Sip_Message::get_header_value_no(int type, int i)
{
    int headerindex = 0;
    int valueindex = 0;
    do{
        SRef<Sip_Header *> h = get_header_of_type(type, headerindex);
        if (h)
        {
            int nval = h->get_no_values();
            if (i < valueindex + nval)	//the value we want is in this header
                return h->get_header_value(i-valueindex);

            valueindex += nval;
            headerindex++;
        }
        else
        {
            SRef<Sip_Header_Value*> nullhdr;
            return nullhdr;
        }
    }while(true);
}

std::list<std::string> Sip_Message::get_route_set()
{
    std::list<std::string> set;
    int n = 0;
    SRef<Sip_Header_Value*> rr;
    while ( ! (rr = get_header_value_no(SIP_HEADER_TYPE_RECORDROUTE,n++)).is_null() )
    {
        set.push_back( ((Sip_Header_Value_Record_Route*)*rr)->get_string_with_parameters() );
    }
    return set;
}

void Sip_Message::set_socket(SRef<Socket*> s)
{
    this->sock = s;
}

SRef<Socket*> Sip_Message::get_socket()
{
    return sock;
}

std::string Sip_Message::get_authenticate_property(std::string prop)
{
    SRef<Sip_Header_Value*> hdr;
    int i = 0;

    do{
        hdr = get_header_value_no(SIP_HEADER_TYPE_WWWAUTHENTICATE, i++);
        if (hdr)
        {
            SRef<Sip_Header_Value_WWW_Authenticate*> whdr = (Sip_Header_Value_WWW_Authenticate*)*hdr;
            if (whdr->has_parameter(prop))
                return unquote(whdr->get_parameter(prop));
        }
    }while(hdr);

    i=0;
    do{
        hdr=get_header_value_no(SIP_HEADER_TYPE_PROXYAUTHENTICATE, i++);
        if (hdr)
        {
            SRef<Sip_Header_Value_Proxy_Authenticate*> phdr = (Sip_Header_Value_Proxy_Authenticate*)*hdr;
            if (phdr->has_parameter(prop))
                return unquote(phdr->get_parameter(prop));
        }
    }while(hdr);
    return "";
}

SRef<Sip_Header_Value_Proxy_Authenticate*> Sip_Message::get_header_value_proxy_authenticate(int i)
{
    SRef<Sip_Header_Value*> hdr;

    hdr=get_header_value_no(SIP_HEADER_TYPE_PROXYAUTHENTICATE, i);
    if (hdr)
    {
        SRef<Sip_Header_Value_Proxy_Authenticate*> phdr = (Sip_Header_Value_Proxy_Authenticate*)*hdr;
        return phdr;
    }
    return NULL;
}

SRef<Sip_Header_Value_Authentication_Info *> Sip_Message::get_header_value_authentication_info(const int &i)
{
    SRef<Sip_Header_Value*> hdr;

    hdr=get_header_value_no(SIP_HEADER_TYPE_AUTHENTICATIONINFO, i);
    if (hdr)
    {
        SRef<Sip_Header_Value_Authentication_Info *> phdr = dynamic_cast<Sip_Header_Value_Authentication_Info *>(*hdr);
        return phdr;
    }
    return NULL;
}

SRef<Sip_Header_Value_WWW_Authenticate*> Sip_Message::get_header_value_wwwauthenticate(int i)
{
    SRef<Sip_Header_Value*> hdr;

    hdr=get_header_value_no(SIP_HEADER_TYPE_WWWAUTHENTICATE, i);
    if (hdr)
    {
        SRef<Sip_Header_Value_WWW_Authenticate*> whdr = (Sip_Header_Value_WWW_Authenticate*)*hdr;
        return whdr;
    }
    return NULL;
}

void Sip_Message::remove_header_value(SRef<Sip_Header_Value*> hval)
{
    int hi = 0;
    SRef<Sip_Header*> hdr;
    for (; hdr = get_header_of_type( hval->get_type(), hi ) ; hi++ )
    {
        for (int vi=0; vi<hdr->get_no_values(); vi++)
        {
            if (hval == hdr->get_header_value(vi) )
            {
                if (hdr->get_no_values()>1)
                    hdr->remove_header_value(vi);
                else
                    remove_header(hdr);
            }
        }
    }
}

void Sip_Message::remove_header(SRef<Sip_Header*> header)
{
    _headers.remove( header );
}

SRef<Sip_Header*> Sip_Message::get_header_of_type(int t, int i)
{
    for (int32_t j=0; j< _headers.size(); j++)
    {
        if ((_headers[j])->get_type() == t)
        {
            if (i==0)
                return _headers[j];
            else
                i--;
        }
    }
    SRef<Sip_Header*> nullhdr;
    return nullhdr;
}

int Sip_Message::parse_headers(const std::string &buf, int startIndex)
{
    int i = startIndex;
    int endBuf = (int)buf.size();
    //This filters the sipfrag messages we receive ... like in NOTIFY ... which most of the times come without any header
    if( startIndex + 4 >= endBuf ) {
#ifdef DEBUG_OUTPUT
        my_dbg("signaling/sip") << "SipMessage::parse_headers: Info: SipMessage without _headers ... only request line" << std::endl;
#endif
        return i;
    }
    do{
        if (i+2<=endBuf && buf[i]=='\n' && buf[i+1]=='\n') {	// i points to first after header
            return i+2;				// return pointer to start of content
        }
        if (i+4<=endBuf && buf[i]=='\r' && buf[i+1]=='\n'
                && buf[i+2]=='\r' && buf[i+3]=='\n' ){	// i points to first after header
            return i+4;				// return pointer to start of content
        }
        if (i+4<=endBuf && buf[i]=='\n' && buf[i+1]=='\r'
                && buf[i+2]=='\n' && buf[i+3]=='\r' ){	// i points to first after header
            return i+4;				// return pointer to start of content
        }
        int eoh = Sip_Utils::find_end_of_header(buf, i);	// i will be adjusted to start of header
        std::string header = buf.substr(i, eoh-i+1);
        // 		my_err << "SipMessage::parse_headers: parsing line = ##" << header
        // 			<< "## [end=" << endBuf << "; i="<< i
        // 			<< "; eoh=" << eoh << "; length="
        // 			<< eoh-i+1 << "]" << end;
        if( header == "" ) {
#ifdef DEBUG_OUTPUT
            my_dbg("signaling/sip") << "SipMessage::parse_headers: Info: Could not copy line to new Message: (empty line)" << std::endl;
#endif
        } else if (!add_line(header)){
#ifdef DEBUG_OUTPUT
            my_dbg("signaling/sip") << "SipMessage::parse_headers: Info: Could not copy line to new Message: " << header << " (unknown)" << std::endl;
#endif
        }
        i=eoh+1;
    }while (i<endBuf);

    return i;
}
