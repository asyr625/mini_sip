#include "sdp_headero.h"


Sdp_HeaderO::Sdp_HeaderO(std::string buildFrom)
    : Sdp_Header(SDP_HEADER_TYPE_O, 2, buildFrom)
{
    size_t len = buildFrom.length();
    if (buildFrom.substr(0,2)!="o="){
#ifdef DEBUG_OUTPUT
        std::cerr << "ERROR: Origin sdp header is not starting with <o=>:"<<buildFrom<< std::endl;
#endif
    }
    size_t i=2;
    while ( buildFrom[i]==' ' && i<len )
        i++;

    username="";
    while ( buildFrom[i]!=' ' && i<len )
        username+=buildFrom[i++];

    while ( buildFrom[i]==' ' && i<len )
        i++;

    session_id="";
    while ( buildFrom[i]!=' ' && i<len )
        session_id+=buildFrom[i++];

    while ( buildFrom[i]==' ' && i<len )
        i++;

    version="";
    while ( buildFrom[i]!=' ' && i<len )
        version+=buildFrom[i++];

    while ( buildFrom[i]==' ' && i<len )
        i++;

    net_type="";
    while ( buildFrom[i]!=' ' && i<len )
        net_type+=buildFrom[i++];

    while ( buildFrom[i]==' ' && i<len )
        i++;

    addr_type="";
    while ( buildFrom[i]!=' ' && i<len )
        addr_type+=buildFrom[i++];

    while ( buildFrom[i]==' ' && i<len )
        i++;

    addr="";
    while ( buildFrom[i]!=' ' && i<len )
        addr+=buildFrom[i++];

}

Sdp_HeaderO::Sdp_HeaderO(std::string username_, std::string session_id_, std::string version_,
                         std::string net_type_, std::string addr_type_, std::string addr_)
    : Sdp_Header(SDP_HEADER_TYPE_O, 2)
{
    this->username=username_;
    this->session_id=session_id_;
    this->version=version_;
    this->net_type=net_type_;
    this->addr_type = addr_type_;
    this->addr=addr_;
}

Sdp_HeaderO::~Sdp_HeaderO()
{
}

std::string Sdp_HeaderO::get_username() const
{
    return username;
}

void Sdp_HeaderO::set_username(std::string usern)
{
    _string_representation_up2date = false;
    this->username = usern;
}

std::string Sdp_HeaderO::get_session_id() const
{
    return session_id;
}

void Sdp_HeaderO::set_session_id(std::string sess_id)
{
    _string_representation_up2date = false;
    this->session_id = sess_id;
}

std::string Sdp_HeaderO::get_version() const
{
    return version;
}

void Sdp_HeaderO::set_version(std::string ver)
{
    _string_representation_up2date = false;
    this->version = ver;
}

std::string Sdp_HeaderO::get_net_type() const
{
    return net_type;
}

void Sdp_HeaderO::set_net_type(std::string net_t)
{
    _string_representation_up2date = false;
    this->net_type = net_t;
}

std::string Sdp_HeaderO::get_addr_type() const
{
    return addr_type;
}

void Sdp_HeaderO::set_addr_type(std::string addr_t)
{
    _string_representation_up2date = false;
    this->addr_type = addr_t;
}

std::string Sdp_HeaderO::get_addr() const
{
    return addr;
}

void Sdp_HeaderO::set_addr(std::string addr_)
{
    _string_representation_up2date = false;
    this->addr = addr_;
}

std::string Sdp_HeaderO::get_string() const
{
    if ( _string_representation_up2date )
        return _string_representation;

    std::string ret = "o=";
    if (username == "")
        ret += "-";
    else
        ret += username;
    ret += " ";
    if (session_id == "")
        ret += "-";
    else
        ret += session_id;
    ret += " ";
    if (version == "")
        ret += "-";
    else
        ret += version;
    ret += " ";
    if (net_type == "")
        ret += "-";
    else
        ret += net_type;
    ret += " ";
    if (addr_type == "")
        ret += "-";
    else
        ret += addr_type;
    ret += " ";
    if (addr == "")
        ret += "-";
    else
        ret += addr;
    return ret;
}
