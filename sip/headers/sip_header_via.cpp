#include "sip_header_via.h"
#include "sip_exception.h"
#include "string_utils.h"

SRef<Sip_Header_Value *> viaFactory(const std::string &build_from)
{
                    return new Sip_Header_Value_Via(build_from);
}

Sip_Header_Factory_Func_Ptr sipHeaderViaFactory = viaFactory;


const std::string sipHeaderValueViaTypeStr = "Via";

Sip_Header_Value_Via::Sip_Header_Value_Via()
    : Sip_Header_Value(SIP_HEADER_TYPE_VIA,sipHeaderValueViaTypeStr)
{
    _port = 0;
    _protocol = "UDP";
    _ip = "UNKNOWN_IP";
}

Sip_Header_Value_Via::Sip_Header_Value_Via(const std::string &build_from)
    : Sip_Header_Value(SIP_HEADER_TYPE_VIA, sipHeaderValueViaTypeStr)
{
    size_t i = 0;
    size_t pos = 0;

    _ip = "";
    _port = 0;
    while ( is_ws(build_from[i]) )
        i++;

    // Parse Via protocol (name and version)
    pos = build_from.find( '/', i );
    if( pos == std::string::npos )
    {
        throw Sip_Exception_Invalid_Message("Sip_Header_Value_Via malformed - via value did not contain version");
    }

    const std::string proto = trim(build_from.substr(i,pos-i));
    if (proto != "SIP")
    {
        throw Sip_Exception_Invalid_Message("Sip_Header_Value_Via malformed - version protocol" );
    }

    i = pos + 1;

    pos = build_from.find( '/', i);
    if( pos == std::string::npos )
    {
        throw Sip_Exception_Invalid_Message("Sip_Header_Value_Via malformed - via value did not contain version");
    }

    const std::string ver = trim(build_from.substr( i, pos - i ));
    if (ver!= "2.0")
    {
        throw Sip_Exception_Invalid_Message("Sip_Header_Value_Via malformed - version unknown" );
    }
    i = pos + 1;


    while (is_ws(build_from[i]))
        i++;

    // Parse Via transport
    pos = build_from.find_first_of(" \t\n\t", i);
    if( pos == std::string::npos )
    {
        throw Sip_Exception_Invalid_Message("Sip_Header_Value_Via malformed - could not determine transport protocol");
    }

    _protocol = trim(build_from.substr( i, pos - i ));
    i = pos + 1;

    pos = build_from.find_first_not_of(" \t\n\r", i);
    if( pos == std::string::npos )
    {
        throw Sip_Exception_Invalid_Message("Sip_Header_Value_Via malformed - could not determine sent-by");
    }
    i = pos;

    size_t start = pos;

    // Parse sent-by host
    // Search for end of host name
    pos = build_from.find_first_of( "[:;, \t\n\r", i );
    if( pos == std::string::npos )
    {
        pos = build_from.size();
    }

    size_t end = pos;

    if( build_from[pos] == '[' )
    {
        // IPv6 address
        start = pos + 1;
        pos = build_from.find( ']', start );
        if( pos == std::string::npos ){
            throw Sip_Exception_Invalid_Message("Sip_Header_Value_Via malformed - could not determine sent-by");
        }

        end = pos;
        pos++;
    }

    _ip = build_from.substr( start, end - start );
    i = pos;

    pos = build_from.find_first_not_of( " \t\n\r", i );
    if( pos != std::string::npos && build_from[pos] == ':' ){
        i = pos + 1;

        i = build_from.find_first_not_of(";, \t\n\r", i );
        if( i == std::string::npos ){
            throw Sip_Exception_Invalid_Message("Sip_Header_Value_Via malformed - could not determine port number");
        }

        pos = build_from.find_first_of(";, \t\n\r", i );
        if( pos == std::string::npos ){
            pos = build_from.length() + i;
        }

        std::string portstr = build_from.substr( i, pos - i );

        _port = atoi( portstr.c_str() );
    }
}

Sip_Header_Value_Via::Sip_Header_Value_Via(const std::string &proto, const std::string &ip, int32_t port)
    : Sip_Header_Value(SIP_HEADER_TYPE_VIA,sipHeaderValueViaTypeStr)
{
    set_protocol(proto);
    set_ip(ip);
    set_port(port);
}

Sip_Header_Value_Via::~Sip_Header_Value_Via()
{
}


std::string Sip_Header_Value_Via::get_string() const
{
    std::string via;
    via = "SIP/2.0/" + _protocol + " ";
    if( _ip.find(':') != std::string::npos )
        // IPv6
        via += '[' + _ip + ']';
    else
        via += _ip;

    if (_port > 0)
        via = via + ":" + itoa( _port );

    return via;
}

std::string Sip_Header_Value_Via::get_protocol() const
{
    return _protocol;
}
void Sip_Header_Value_Via::set_protocol(const std::string &protocol)
{
    _protocol = protocol;
}

std::string Sip_Header_Value_Via::get_ip() const
{
    return _ip;
}
void Sip_Header_Value_Via::set_ip(const std::string &ip)
{
    _ip = ip;
}

int32_t Sip_Header_Value_Via::get_port() const
{
    return _port;
}
void Sip_Header_Value_Via::set_port(int32_t port)
{
    _port = port;
}
