#include "sdp_headera.h"
#include "string_utils.h"
#include <cstring>
#include <iostream>
using namespace std;

Sdp_HeaderA::Sdp_HeaderA(std::string buildFrom)
    : Sdp_Header(SDP_HEADER_TYPE_A, 10, buildFrom)
{
    my_assert(buildFrom.substr(0,2)=="a=");
    _attributes = trim(buildFrom.substr(2, buildFrom.length()-2));
}

Sdp_HeaderA::~Sdp_HeaderA()
{
}


std::string Sdp_HeaderA::get_attributes() const
{
    return _attributes;
}
void Sdp_HeaderA::set_attributes(std::string attr)
{
    _string_representation_up2date = false;
    this->_attributes = attr;
}

std::string Sdp_HeaderA::get_string() const
{
    if( _string_representation_up2date )
        return _string_representation;
    else
        return "a=" + _attributes;
}

std::string Sdp_HeaderA::get_attribute_type() const
{
    size_t pos;

    if(( pos = _attributes.find( ":" )) == std::string::npos )
        return "property";
    return _attributes.substr( 0, pos );
}
std::string Sdp_HeaderA::get_attribute_value() const
{
    if( get_attribute_type() == "property" )
        return _attributes;
    uint32_t pos = (int)_attributes.find( ":" );
    if( _attributes.length() <= pos + 1 )
    {
        cerr << "Invalid a field in SDP packet" << endl;
        return "";
    }
    return _attributes.substr( pos + 1, _attributes.length() - 2 );
}

void Sdp_HeaderA::get_att_from_file_selector()
{
    size_t pos;
    size_t pos2;

    name = false;
    type = false;
    size = false;
    hash = false;

    pos = 14;
    pos2 = 14;

    int searchname;
    int searchtype;
    int searchsize;
    int poshash;
    int iter;
    int counter = 0;

    for(iter = 15; iter<(int)strlen(_attributes.c_str()); iter++)
    {
        if(_attributes[iter] == ':')
            counter++;
    }
    //cerr<<counter<<endl;

    while(counter != 1)
    {
        counter --;
        pos2 = _attributes.find( ":",pos);

        //cerr<<pos2<<endl;

        pos = pos2 - 4;

        //cerr<<( attributes.substr(pos, 4) )<<endl;

        if( _attributes.substr(pos, 4) == "name")
        {
            name = true;
            searchname = (int)_attributes.find('"',pos2+2);
            //cerr<<searchname<<endl;
            filename = ( _attributes.substr(pos2+2,(searchname-pos2-2)) );
            //cerr<<filename<<endl;
        }
        if( _attributes.substr(pos, 4) == "type")
        {
            type = true;
            searchtype = (int)_attributes.find(" ",pos2);
            //cerr<<searchtype<<endl;
            filetype = ( _attributes.substr(pos2+1,(searchtype-pos2)) );
            //cerr<<filetype<<endl;
        }
        if( _attributes.substr(pos, 4) == "size"){
            size = true;
            searchsize = (int)_attributes.find(" ", pos2);
            //cerr<<searchsize<<endl;
            filesizes = ( _attributes.substr(pos2+1,(searchsize-pos2)) );
            //cerr<<filesizes<<endl;
        }
        if( _attributes.substr(pos, 4) == "hash")
        {
            hash = true;
            poshash = (int)_attributes.find( ":", pos2+1);
            hashused = ( _attributes.substr(pos2+1,poshash-pos2-1));
            hashforfile = ( _attributes.substr(poshash+1,(strlen(_attributes.c_str())-poshash)) );
            //cerr<<"used hash "<<hashused<<endl;
            //cerr<<"hash "<<hashforfile<<endl;
        }

        pos2 ++;
        pos = pos2;
    }
}

std::string Sdp_HeaderA::get_rtp_map(int format) const
{

}
