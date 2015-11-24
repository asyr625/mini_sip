#include "sip_uri.h"
#include "dbg.h"
#include "string_utils.h"

#include <stdlib.h>

#include <vector>
#include <locale>

#ifdef DEBUG_OUTPUT
#include <iostream>
#endif

using namespace std;

Sip_Uri::Sip_Uri(std::string build_from)
{
    set_uri(build_from);
}

Sip_Uri::~Sip_Uri()
{
}

void Sip_Uri::clear()
{
    this->_display_name = "";
    this->_protocol_id = "sip";
    this->_user_name = "";
    this->_ip = "";
    this->_port = 0;
    this->_valid_uri = false;
    this->_parameters.clear();
}

void Sip_Uri::set_uri( std::string buildFrom )
{
    size_t pos;
    string UriData;
    char paramDelimiter = '\0';

    clear();

#ifdef DEBUG_OUTPUT
    my_dbg("signaling/sip") << "SipUri::fromString = " << buildFrom << endl;
#endif

    //look for the full name ...
    pos = buildFrom.find( '<' );
    if( pos != string::npos )
    {
        size_t pos2, pos3;
        pos2 = buildFrom.find( '>' );
        if( pos2 == string::npos ) {
#ifdef DEBUG_OUTPUT
            cerr << "Sip_Uri::constructor - bogus Uri ... " << buildFrom << endl;
#endif
            return;
        }
        //process the full name ...
        string nameTmp;
        nameTmp = trim( buildFrom.substr( 0, pos ) );
        pos3 = nameTmp.find( '"' );
        while( pos3 != string::npos )
        {
            nameTmp.erase( pos3, 1 );
            pos3 = nameTmp.find( '"' );
        }
        set_display_name( nameTmp );
        buildFrom.erase( 0, pos + 1 ); //remove the full name ...
        //remove the leftovers (ZZZ)... XXX<YYY>ZZZ
        pos2 = buildFrom.find( '>' );
        buildFrom.erase( pos2 );
    }

    //now we process the stuff that was between the < and > chars ...

    //separate the params from the Uri ...
    if( (pos = buildFrom.find( ';' )) != string::npos )
    {
        UriData = buildFrom.substr( 0, pos );
        buildFrom.erase( 0, pos );
        paramDelimiter = ';';
    }
    else if( (pos = buildFrom.find( '?' )) != string::npos )
    {
        UriData = buildFrom.substr( 0, pos );
        buildFrom.erase( 0, pos );
        paramDelimiter = '?';
    } else {
        UriData = buildFrom;
    }

    //parse the Uri info related to user (protocol, userName, ip, port)
    parse_user_info( UriData );

    //now parse the parameters ...
    if( paramDelimiter != '\0' )
    {
        std::vector<string> params;
        string paramName;
        unsigned int idx;
        params = split( buildFrom, true, paramDelimiter );
        for( idx = 0; idx < params.size(); idx++ )
        {
            pos = params[idx].find( '=' );
            if( pos != string::npos )
            {
                paramName = params[idx].substr( 0, pos );
                params[idx].erase( 0, pos + 1 );
            } else {
                paramName = params[idx];
                params[idx] = "";
            }

            set_parameter( paramName, params[ idx ] );
        }
    }

    _valid_uri = true;
}

void Sip_Uri::parse_user_info( std::string UriData )
{
    //Lets piece the Uri (without params first ) ... in UriData string
    size_t pos;
#ifdef DEBUG_OUTPUT
// 	cerr << "SipUri::parseUserInfo - " << UriData << endl;
#endif
    //first identify the protocol ...
    if( UriData.substr(0,4) == "sip:" ) {
        set_protocol_id( "sip" );
        UriData.erase( 0, 4 );
    } else 	if( UriData.substr(0,4) == "tel:" ) {
        set_protocol_id( "tel" );
        UriData.erase( 0, 4 );
    } else 	if( UriData.substr(0,5) == "sips:" ) {
        set_protocol_id( "sips" );
        UriData.erase( 0, 5 );
    }

    //try to get the username ...
    pos = UriData.find( '@' );
    if( pos != string::npos )
    { //there is a username ...
        _user_name = UriData.substr( 0, pos );
        UriData.erase( 0, pos + 1 );
    } else { //no user info ...
        _user_name = "";
    }

    if( UriData[0] == '[' )
    {
        pos = UriData.find(']');

        if( pos != string::npos ){
            set_ip( UriData.substr( 1, pos - 1 ) );

            if( UriData[ pos + 1 ] == ':' ) { //there is port info ...
                UriData.erase( 0, pos + 2);
                set_port( atoi(UriData.c_str()) );
            } else {
                set_port( 0 );
            }
            return;
        }
    }

    //now, we get the host/ip ...
    pos = UriData.find( ':' );
    if( pos != string::npos ) { //there is port info ...
        set_ip( UriData.substr( 0, pos ) );
        UriData.erase( 0, pos + 1);
        set_port( atoi(UriData.c_str()) );
    } else {
        set_ip( UriData );
        UriData.erase( 0, pos );
        set_port( 0 );
    }
}

void Sip_Uri::set_params(std::string userName, std::string ip_, std::string type, int port_)
{
    clear();

#ifdef DEBUG_OUTPUT
// 	cerr << "SipUri::setParams " << endl;
#endif
    parse_user_info( userName );
    if( get_user_name() == "" && get_ip() != "" )
    {
        set_user( get_ip() );
        set_ip( "" );
        set_port( 0 );
    }

    if( get_ip() == "" && ip_ != "" )
    {
        set_ip( ip_ );
    }

    if( port_ != 0 ) set_port( port_ );
    if( type != "" ) set_user_type( type );
    _valid_uri = true;
}

std::string Sip_Uri::get_string() const
{
    string Uri = "";

    if( !is_valid() )
    {
#ifdef DEBUG_OUTPUT
        cerr << "SipUri::getString - invalid Uri!" << endl;
#endif
        return "";
    }

    if( get_display_name() != "" )
    {
        Uri += "\"" + get_display_name() + "\" ";
    }
    Uri += "<";
    Uri += get_request_uri_string();
    Uri += ">";

#ifdef DEBUG_OUTPUT
// 	cerr << "SipUri::getString() - " << Uri << endl << endl;
#endif
    return Uri;
}

std::string Sip_Uri::get_user_ip_string() const
{
    string Uri = "";

    if( !is_valid() )
    {
#ifdef DEBUG_OUTPUT
        cerr << "SipUri::get_user_ip_string - invalid Uri!" << endl;
#endif
        return "";
    }

    if( get_user_name() != "" ) Uri += get_user_name() + "@";

    if( get_ip().find(':') != string::npos )
    {
        // IPv6
        Uri += '[' + get_ip() + ']';
    }
    else
    {
        Uri += get_ip();
    }

#ifdef DEBUG_OUTPUT
// 	cerr << "SipUri::getUserIpString() - " << Uri << endl;
#endif
    return Uri;
}

std::string Sip_Uri::get_request_uri_string() const
{
    string Uri = "";

    if( !is_valid() ) {
#ifdef DEBUG_OUTPUT
        cerr << "SipUri::getString - invalid Uri!" << endl;
#endif
        return "";
    }

    if( get_protocol_id() != "" )
         Uri += get_protocol_id() + ":";
    Uri += get_user_ip_string();
    if( get_port() != 0 )
    {
        Uri += ":" + itoa( _port );
    }

    map<string, string>::const_iterator iter;
    map<string, string>::const_iterator last = _parameters.end();

    for( iter = _parameters.begin(); iter != last; iter++ )
    {
        string key = iter->first;
        string val = iter->second;

        if( val.empty() )
            Uri += ";" + key ;
        else
            Uri += ";" + key + "=" + val;
    }

#ifdef DEBUG_OUTPUT
// 	cerr << "SipUri::get_request_uri_string() - " << Uri << endl;
#endif
    return Uri;
}


void Sip_Uri::set_display_name(std::string id)
{
    _display_name = id;
}

const std::string & Sip_Uri::get_display_name() const
{
    return _display_name;
}

void Sip_Uri::set_protocol_id(std::string protocol_id)
{
    _protocol_id = protocol_id;
}

const std::string & Sip_Uri::get_protocol_id() const
{
    return _protocol_id;
}

void Sip_Uri::set_user(std::string id)
{
    _user_name = id;
}

const std::string & Sip_Uri::get_user_name() const
{
    return _user_name;
}

void Sip_Uri::set_ip(std::string ip)
{
    if( ip[0] == '[' && ip[ip.length()-1] == ']' )
        this->_ip = ip.substr( 1, ip.length() - 2 );
    else
        this->_ip = ip;
}

const std::string & Sip_Uri::get_ip() const
{
    return _ip;
}

void Sip_Uri::set_port(int port)
{
    _port = port;
}

int Sip_Uri::get_port() const
{
    return _port;
}

void Sip_Uri::set_user_type(std::string user_type)
{
    set_parameter( "user", user_type );
}

const std::string & Sip_Uri::get_user_type() const
{
    return get_parameter("user");
}

void Sip_Uri::set_transport(std::string transp)
{
    set_parameter( "transport", transp );
}

const std::string & Sip_Uri::get_transport() const
{
    return get_parameter("transport");
}

void Sip_Uri::set_parameter(const std::string &key, const std::string &val)
{
    _parameters[key] = val;
}

bool Sip_Uri::has_parameter(const std::string &key) const
{
    std::map<std::string, std::string>::const_iterator iter = _parameters.find(key);
    return iter != _parameters.end();
}

const std::string & Sip_Uri::get_parameter(const std::string &key) const
{
    static const std::string empty = "";

    std::map<std::string, std::string>::const_iterator iter = _parameters.find(key);
    if( iter != _parameters.end() )
        return iter->second;
    else
        return empty;
}

void Sip_Uri::remove_parameter(const std::string &key)
{
    _parameters.erase(key);
}

bool Sip_Uri::is_valid() const
{
    return _valid_uri;
}

void Sip_Uri::make_valid( bool valid )
{
    _valid_uri = valid;
}


int Sip_Uri::operator==( const Sip_Uri &uri ) const
{
    locale loc("");

    if( get_protocol_id() != uri.get_protocol_id() ||
        get_user_name() != uri.get_user_name() ||
        str_case_cmp(get_ip(), uri.get_ip(), loc) ||
        get_port() != uri.get_port() )
    {
        return false;
    }

    // RFC 3261 19.1.4
    // A URI omitting any component with a default value will not
        // match a URI explicitly containing that component with its
        // default value.
    const char *keys[] = { "transport", "user", "ttl",
                  "method", "maddr", NULL };

    for( int j = 0; keys[j]; j++ )
    {
        const char *key = keys[j];
        const string & value = get_parameter( key );

        if( str_case_cmp( value, uri.get_parameter( key ), loc) )
        {
            return false;
        }
    }

    map<string,string>::const_iterator i;
    map<string,string>::const_iterator last = _parameters.end();

    for( i = _parameters.begin(); i != last; i++ )
    {
        const string &key = i->first;
        const string &value = i->second;

        if( uri.has_parameter( key ) )
        {
            if( str_case_cmp( value, uri.get_parameter( key ), loc ) )
            {
                return false;
            }
        }
    }

    last = uri._parameters.end();

    for( i = uri._parameters.begin(); i != last; i++ )
    {
        const string &key = i->first;
        const string &value = i->second;

        if( has_parameter( key ) ){
            if( str_case_cmp( value, get_parameter( key ), loc ) )
            {
                return false;
            }
        }
    }
    return true;
}


std::ostream& operator << (std::ostream& os, const Sip_Uri& uri)
{
    return os << uri.get_string();
}
