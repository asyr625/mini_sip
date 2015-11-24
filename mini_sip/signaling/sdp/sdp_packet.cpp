#include "sdp_packet.h"
#include "string_utils.h"

#include "sdp_headero.h"
#include "sdp_headerv.h"
#include "sdp_headers.h"
#include "sdp_headert.h"

#include <iostream>
using namespace std;

SRef<Sip_Message_Content*> sdpSipMessageContentFactory(const std::string & buf, const std::string & ContentType)
{
    return new Sdp_Packet(buf);
}

Sdp_Packet::Sdp_Packet()
{
}

Sdp_Packet::Sdp_Packet(std::string build_from)
{
    SRef<Sdp_HeaderM *> lastM = NULL;
    SRef<Sdp_HeaderA *> attr = NULL;
    std::vector<std::string> lines = split_lines(build_from);

    for (uint32_t i = 0; i < lines.size(); i++)
    {
        switch (lines[i][0])
        {
        case 'v':
            add_header(SRef<Sdp_Header*>(new Sdp_HeaderV(lines[i])));
            break;
        case 'o':
            add_header(SRef<Sdp_Header*>(new Sdp_HeaderO(lines[i])));
            break;
        case 's':
            add_header(SRef<Sdp_Header*>(new Sdp_HeaderS(lines[i])));
            break;
        case 'b': {
            SRef<Sdp_HeaderB*> b = new Sdp_HeaderB(lines[i]);
            if( lastM )
                lastM->set_bandwidth(b);
            else
                add_header(*b);
            break;
        }
        case 'i':{
            SRef<Sdp_HeaderI*> in = new Sdp_HeaderI(lines[i]);
            if( lastM )
                lastM->set_information(in);
            else
                add_header(*in);
            break;
        }
        case 'c':{
            SRef<Sdp_HeaderC*> c = new Sdp_HeaderC(lines[i]);
            if( lastM )
                lastM->set_connection(c);
            else
                add_header(*c);
            break;
        }
        case 't':
            add_header(SRef<Sdp_Header*>(new Sdp_HeaderT(lines[i])));
            break;
        case 'm':
            lastM=new Sdp_HeaderM(lines[i]);
            add_header(*lastM);
            break;
        case 'a':
            attr=new Sdp_HeaderA(lines[i]);
            if(lastM)
            {
                // media level attribute
                lastM->add_attribute(attr);
            }
            else
            {
                // Session level attribute
                add_header(*attr);
            }
            break;
        default:
            cerr << "ERROR: unknown SDP header: "<< lines[i]<< endl;
            break;
        }
    }
}


SRef<Sdp_HeaderC*> Sdp_Packet::get_session_level_connection()
{
    for(unsigned i = 0; i < headers.size(); i++)
    {
        if ( headers[i].is_null() )
            cerr <<"WARNING: sdp header is null"<< endl;
        if ((headers[i])->get_type() == SDP_HEADER_TYPE_C)
        {
            SRef<Sdp_HeaderC*> cptr;
            cptr = SRef<Sdp_HeaderC*>((Sdp_HeaderC *)*(headers[i]));
            return cptr;
        }
    }
    return NULL;
}

std::string Sdp_Packet::get_key_mgmt()
{
    std::string message;
    SRef<Sdp_HeaderA*> aptr;

    for( unsigned i=0; i<headers.size();i++ )
    {
        if( headers[i].is_null() )
            cerr << "WARNING: sdp header is null" << endl;
        if((headers[i])->get_type() == SDP_HEADER_TYPE_A )
        {
            aptr = SRef<Sdp_HeaderA*>((Sdp_HeaderA *)*(headers[i]));
            if( aptr->get_attribute_type() == "key-mgmt" )
                return aptr->get_attribute_value();
        }
    }

    return "";
}

void Sdp_Packet::add_header(SRef<Sdp_Header*> h)
{
    headers.push_back(h);
    if (h->get_type()==SDP_HEADER_TYPE_A)
        h->set_priority(6);
}

std::string Sdp_Packet::get_string() const
{
    std::string ret = "";
    std::list<SRef<Sdp_HeaderA*> >::iterator  iAttr;
    std::list<SRef<Sdp_HeaderA*> > attributes;
    for (int32_t prio = 0; prio < 15; prio++)
        for (unsigned i=0; i< headers.size(); i++)
            if (headers[i]->get_priority()==prio)
            {
                ret+="\r\n"+headers[i]->get_string();
                if (headers[i]->get_type() == SDP_HEADER_TYPE_M)
                {
                    attributes = ((Sdp_HeaderM *)*(headers[i]))->get_attributes();
                    for(iAttr = attributes.begin(); iAttr!=attributes.end(); iAttr++)
                    {
                        ret+="\r\n"+(*iAttr)->get_string();
                    }
                }
            }
    return trim(ret)+"\r\n";
}


std::vector<SRef<Sdp_Header*> > Sdp_Packet::get_headers()
{
    return headers;
}

std::string Sdp_Packet::get_format_match(Sdp_Packet &pack)
{
    SRef<Sdp_HeaderM*> mym;
    SRef<Sdp_HeaderM*> otherm;

    unsigned int i;

    for (i = 0 ; i< headers.size(); i++)
        if ((headers[i])->get_type() == SDP_HEADER_TYPE_M)
            mym = SRef<Sdp_HeaderM*>((Sdp_HeaderM*)(*headers[i]));

    for (i=0; i<pack.headers.size(); i++)
        if ((headers[i])->get_type() == SDP_HEADER_TYPE_M)
            otherm = SRef<Sdp_HeaderM*>((Sdp_HeaderM*)(*pack.headers[i]));

    if (mym.is_null() || otherm.is_null() )
    {
        cerr << "ERROR: BUG: SDP packet did not contain <m> header. Defaulting to PCMu"<< endl;
        return 0;
    }


    for (i=0; (int)i< mym->get_nr_formats(); i++)
        for (int32_t j=0; j< otherm->get_nr_formats(); j++)
            if (mym->get_format(i) == otherm->get_format(j))
                return mym->get_format(i);
    cout << "ERROR: could not match any codec format - trying anyway with PCMu"<< endl;
    return 0;
}

std::string Sdp_Packet::get_first_media_format()
{
    SRef<Sdp_HeaderM*> mym;

    for (unsigned i = 0 ; i< headers.size(); i++)
        if ((headers[i])->get_type() == SDP_HEADER_TYPE_M)
            mym = SRef<Sdp_HeaderM*>((Sdp_HeaderM*)(*headers[i]));

    if (mym.is_null() || mym->get_nr_formats()<1)
    {
        cerr << "ERROR: BUG: SDP packet did not contain <m> header. Defaulting to PCMu"<< endl;
        return 0;
    }
    return mym->get_format(0);
}

bool Sdp_Packet::media_format_available(std::string f)
{
    SRef<Sdp_HeaderM*> mym;

    unsigned int i;
    for (i = 0 ; i< headers.size(); i++)
        if ((headers[i])->get_type() == SDP_HEADER_TYPE_M)
            mym = SRef<Sdp_HeaderM*>((Sdp_HeaderM*)(*headers[i]));

    if (mym.is_null() || mym->get_nr_formats()<1)
    {
        cerr << "ERROR: BUG: SDP packet did not contain <m> header. Defaulting to PCMu"<< endl;
        return 0;
    }

    for (i = 0;  (int)i < mym->get_nr_formats(); i++)
        if (mym->get_format(i) == f)
            return true;
    return false;
}

void Sdp_Packet::set_session_level_attribute(std::string type, std::string value)
{
    SRef<Sdp_HeaderA*> a = new Sdp_HeaderA("a=X");
    a->set_priority(6);
    a->set_attributes(type+":"+value);
    add_header(SRef<Sdp_Header*>(*a));
}

std::string Sdp_Packet::get_session_level_attribute(std::string type)
{
    SRef<Sdp_HeaderA*> a;
    std::string ret = "";

    for (unsigned i = 0 ; i< headers.size(); i++)
    {
        //search all headers for type A
        if ((headers[i])->get_type() == SDP_HEADER_TYPE_A)
        {
            a = SRef<Sdp_HeaderA*>((Sdp_HeaderA*)(*headers[i]));

            if(a->get_attribute_type()==type)
            {
                ret=a->get_attribute_value();
                break;
            }
        }
    }
    return ret;
}

void Sdp_Packet::remove_last_header()
{
    headers.pop_back();
}
