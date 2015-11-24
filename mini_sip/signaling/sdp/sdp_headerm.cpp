#include "sdp_headerm.h"

#include <stdlib.h>
#include "string_utils.h"

Sdp_HeaderM::Sdp_HeaderM(std::string buildFrom)
    : Sdp_Header(SDP_HEADER_TYPE_M, 8, buildFrom)
{
    size_t len = buildFrom.length();
    if (buildFrom.substr(0,2)!="m="){
#ifdef DEBUG_OUTPUT
        cerr << "ERROR: Origin sdp header is not starting with <m=>"<< endl;
#endif
    }
    size_t i=2;
    while (buildFrom[i] == ' ' && i < len)
        i++;

    media="";
    while (buildFrom[i] != ' ' && i < len)
        media += buildFrom[i++];

    while (buildFrom[i] == ' ' && i < len)
        i++;

    std::string portstr = "";
    while (buildFrom[i] != ' ' && i < len)
        portstr += buildFrom[i++];

    int32_t np = 0;
    for (unsigned j = 0; j < portstr.length(); j++)
        if (portstr[j] == '/')
            np = j;
    if (np > 0)
    {
        port = atoi(portstr.substr(0,np).c_str());
        nPorts= atoi(portstr.substr(np+1,portstr.length()-(np+1)-1).c_str());
    }
    else
    {
        port = atoi(portstr.c_str());
        nPorts=1;
    }

    while (buildFrom[i]==' ' && i<len)
        i++;

    transport = "";
    while (buildFrom[i]!=' ' && i<len)
        transport += buildFrom[i++];

    bool done = false;
    while (!done)
    {
        while (buildFrom[i]==' '  && i < len )
            i++;

        std::string f = "";
        while (buildFrom[i] != ' ' && i < len )
            f+=buildFrom[i++];

        if (f.length()>0)
            formats.push_back( f );

        if ( i >= len )
            done = true;
    }
}

Sdp_HeaderM::Sdp_HeaderM(std::string media_, int32_t port_, int32_t n_ports, std::string transp)
    : Sdp_Header(SDP_HEADER_TYPE_M, 8)
{
    this->media=media_;
    this->port=port_;
    this->nPorts=n_ports;
    this->transport=transp;
}

Sdp_HeaderM::Sdp_HeaderM(const Sdp_HeaderM &src)
    : Sdp_Header( SDP_HEADER_TYPE_M, 8 )
{
    *this = src;
}

Sdp_HeaderM::~Sdp_HeaderM()
{
}

Sdp_HeaderM &Sdp_HeaderM::operator=(const Sdp_HeaderM &src)
{
    _string_representation_up2date = false;
    set_priority( src.get_priority() );
    media = src.media;
    port = src.port;
    nPorts = src.nPorts;
    transport = src.transport;
    formats = src.formats;
    bandwidth = src.bandwidth;
    attributes.clear();

    std::list<SRef<Sdp_HeaderA *> >::const_iterator i;

    for(i = src.attributes.begin(); i != src.attributes.end(); i++)
        add_attribute( new Sdp_HeaderA( ***i ) );

    return *this;
}

void Sdp_HeaderM::add_format(std::string f)
{
    _string_representation_up2date = false;
    formats.push_back(f);
}

int32_t Sdp_HeaderM::get_nr_formats() const
{
    return (int32_t)formats.size();
}

std::string Sdp_HeaderM::get_format(int32_t i) const
{
    return formats[i];
}

std::string Sdp_HeaderM::get_media() const
{
    return media;
}

void Sdp_HeaderM::set_media(std::string m)
{
    _string_representation_up2date = false;
    this->media = m;
}

int32_t Sdp_HeaderM::get_port() const
{
    return port;
}
void Sdp_HeaderM::set_port(int32_t p)
{
    _string_representation_up2date = false;
    this->port = p;
}

int32_t Sdp_HeaderM::get_nr_ports() const
{
    return nPorts;
}
void Sdp_HeaderM::set_nr_ports(int32_t n)
{
    _string_representation_up2date = false;
    this->nPorts = n;
}

std::string Sdp_HeaderM::get_transport() const
{
    return transport;
}

void Sdp_HeaderM::set_transport(std::string t)
{
    _string_representation_up2date = false;
    this->transport = t;
}

std::string Sdp_HeaderM::get_string() const
{
    if (_string_representation_up2date)
        return _string_representation;

    std::string ret = "m=" + media + " ";

    if (nPorts > 1)
        ret += port + "/" + itoa(nPorts);
    else
        ret += itoa(port);

    ret += " " + transport;

    for (unsigned i = 0; i < formats.size(); i++)
        ret += " " + formats[i];

    if( connection )
        ret += "\r\n" + connection->get_string();

    if(bandwidth)
        ret += "\r\n" + bandwidth->get_string();

    if( information )
        ret += "\r\n" + information->get_string();

    return ret;
}

void Sdp_HeaderM::add_attribute(SRef<Sdp_HeaderA*> a)
{
    _string_representation_up2date = false;
    attributes.push_back(a);
}

std::string Sdp_HeaderM::get_attribute(std::string key, uint32_t n) const
{
    std::list<SRef<Sdp_HeaderA *> >::const_iterator i;
    uint32_t nAttr = 0;

    for(i = attributes.begin(); i != attributes.end(); i++)
    {
        if((*i)->get_attribute_type() == key)\
        {
            if(nAttr == n)
                return (*i)->get_attribute_value();
            nAttr++;
        }
    }
    return "";
}

std::list<SRef<Sdp_HeaderA*> > Sdp_HeaderM::get_attributes() const
{
    return attributes;
}

std::string Sdp_HeaderM::get_rtp_map(std::string format) const
{
    int i = 0;
    std::string attrib;
    std::string value = "rtpmap";

    while((attrib = get_attribute(value, i)) != "")
    {
        size_t firstSpace = attrib.find(" ");
        //cerr << "SdpHeaderM::getRtpMap - value retrieved = " << attrib << "; substr = " << attrib.substr(0, firstSpace) << endl;
        if( attrib.substr(0, firstSpace) == format )
            return attrib.substr(firstSpace + 1, attrib.size());

        i++;
    }
    return "";
}

std::string Sdp_HeaderM::get_fmtp_param(std::string format) const
{
    int i = 0;
    std::string attrib;
    std::string value = "fmtp";

    while((attrib = get_attribute(value, i)) != "")
    {
        size_t firstSpace = attrib.find(" ");
        //cerr << "SdpHeaderM::getFmtpParam - value retrieved = " << attrib << "; substr = " << attrib.substr(0, firstSpace) << endl;
        if( attrib.substr(0, firstSpace) == format )
        {
            return attrib.substr(firstSpace + 1, attrib.size());
        }
        i++;
    }
    return "";
}

void Sdp_HeaderM::set_bandwidth( SRef<Sdp_HeaderB*> b )
{
    _string_representation_up2date = false;
    bandwidth = b;
}

SRef<Sdp_HeaderB*> Sdp_HeaderM::get_bandwidth() const
{
    return bandwidth;
}

void Sdp_HeaderM::set_connection( SRef<Sdp_HeaderC*> c )
{
    _string_representation_up2date = false;
    connection = c;
}

SRef<Sdp_HeaderC*> Sdp_HeaderM::get_connection() const
{
    return connection;
}

void Sdp_HeaderM::set_information(SRef<Sdp_HeaderI*> i)
{
    _string_representation_up2date = false;
    information = i;
}

SRef<Sdp_HeaderI*> Sdp_HeaderM::get_information() const
{
    return information;
}

void Sdp_HeaderM::copy_attributes ( SRef <Sdp_HeaderM * > m , bool changing_direction)
{
    std::list<SRef<Sdp_HeaderA *> > attrList =  m->get_attributes();
    std::list<SRef<Sdp_HeaderA *> > :: iterator iter = attrList.begin() ;
    for ( ; iter != attrList.end() ;  iter ++ )
    {
        SRef<Sdp_HeaderA*> attr = *(iter);
        SRef<Sdp_HeaderA*> a = new Sdp_HeaderA("a=X");
        if ( changing_direction )
        {
            if ( attr->get_string().compare("a=recvonly") == 0 )  { a->set_attributes("sendonly");}
            else
            {
                if ( attr->get_string().compare("a=sendonly") == 0 )
                    a->set_attributes("recvonly");
                else
                    a->set_attributes(  attr->get_attributes());
            }
        }
        else
        {
            a->set_attributes(  attr->get_attributes());
        }
        this->add_attribute( *a);
    }
}

std::string Sdp_HeaderM::get_direction_attribute() const
{
    std::list<SRef<Sdp_HeaderA *> > attrList =  get_attributes();
    std::list<SRef<Sdp_HeaderA *> > :: iterator iter = attrList.begin() ;

    for ( ; iter != attrList.end() ;  iter ++ )
    {
        SRef<Sdp_HeaderA*> attr = *(iter);
        if ( attr->get_string().compare("a=recvonly") == 0 ) return attr->get_string();
        if ( attr->get_string().compare("a=sendonly") == 0 ) return attr->get_string();
        if ( attr->get_string().compare("a=sendrecv") == 0 ) return attr->get_string();
        if ( attr->get_string().compare("a=inactive") == 0 ) return attr->get_string();
    }
    return " ";
}

void Sdp_HeaderM::set_direction_attribute(std :: string str )
{
    _string_representation_up2date = false;
    str_attr.assign(str);
    bool setted = false;
    std::list<SRef<Sdp_HeaderA *> > attrList =  get_attributes();
    std::list<SRef<Sdp_HeaderA *> > :: iterator iter = attrList.begin() ;

    for ( ; iter != attrList.end() ;  iter ++ )
    {
        SRef<Sdp_HeaderA*> attr = *(iter);
        if ( attr->get_string().compare("a=recvonly") == 0 || attr->get_string().compare("a=sendonly") == 0 ||
             attr->get_string().compare("a=sendrecv") == 0 || attr->get_string().compare("a=inactive") == 0 )
        {
            attr->set_attributes(str_attr);
            setted = true;
        }
    }
    if ( !setted )
    {
        SRef<Sdp_HeaderA*> attr = new Sdp_HeaderA ( "a=" + str_attr);
        this->add_attribute(*attr);
    }
}

void Sdp_HeaderM::swap_direction_attribute ()
{
    _string_representation_up2date = false;
    std :: string directionStr =  this->get_direction_attribute() ;
    if ( directionStr.compare("a=recvonly") == 0 )
    {
        this->set_direction_attribute("sendonly");
    }
    if ( directionStr.compare("a=sendonly") == 0 )
    {
        this->set_direction_attribute("recvonly");
    }
}
