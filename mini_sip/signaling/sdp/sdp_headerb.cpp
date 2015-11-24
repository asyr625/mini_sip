#include "sdp_headerb.h"
#include <stdlib.h>
#include <iostream>
using namespace std;

Sdp_HeaderB::Sdp_HeaderB(const std::string &buildFrom)
    : Sdp_Header( SDP_HEADER_TYPE_B, 5, buildFrom ), ct(0), as(0), tias(0)
{
    if (buildFrom.substr(0,2)!="b="){
        std::cerr << "ERROR: Bandwidth sdp header is not starting with 'b='"<< std::endl;
    }

    size_t front = buildFrom.find("CT");
    if(front != buildFrom.npos)
    {
        while(front != buildFrom.npos && buildFrom[front++] != ':');
        size_t rear = buildFrom.find(' ', front);
        try
        {
            ct = atoi(buildFrom.substr(front, rear).c_str());
        }
        catch(...) {}
        return;
    }
    front = buildFrom.find("TIAS");
    if(front != buildFrom.npos)
    {
        while(front != buildFrom.npos && buildFrom[front++] != ':');
        size_t rear = buildFrom.find(' ', front);
        try
        {
            tias = atoi(buildFrom.substr(front, rear).c_str());
        }
        catch(...) {}
        return;
    }
    front = buildFrom.find("AS");
    if(front != buildFrom.npos)
    {
        while(front != buildFrom.npos && buildFrom[front++] != ':');
        size_t rear = buildFrom.find(' ', front);
        try
        {
            as = atoi(buildFrom.substr(front, rear).c_str());
        }
        catch(...) {}
        return;
    }
    front = buildFrom.find("RS");
    if(front != buildFrom.npos)
    {
        while(front != buildFrom.npos && buildFrom[front++] != ':');
        size_t rear = buildFrom.find(' ', front);
        try
        {
            rs = atoi(buildFrom.substr(front, rear).c_str());
        }
        catch(...) {}
        return;
    }
    front = buildFrom.find("RR");
    if(front != buildFrom.npos)
    {
        while(front != buildFrom.npos && buildFrom[front++] != ':');
        size_t rear = buildFrom.find(' ', front);
        try
        {
            rr = atoi(buildFrom.substr(front, rear).c_str());
        }
        catch(...) {}
        return;
    }
}

Sdp_HeaderB::~Sdp_HeaderB()
{
}

unsigned int Sdp_HeaderB::get_ct() const
{
    return ct;
}

void Sdp_HeaderB::set_ct(const unsigned int &_ct)
{
    _string_representation_up2date = false;
    ct = _ct;
}

unsigned int Sdp_HeaderB::get_as() const
{
    return as;
}

void Sdp_HeaderB::set_as(const unsigned int &_as)
{
    _string_representation_up2date = false;
    as = _as;
}

unsigned int Sdp_HeaderB::get_tias() const
{
    return tias;
}

void Sdp_HeaderB::set_tias(const unsigned int &_tias)
{
    _string_representation_up2date = false;
    tias = _tias;
}

unsigned int Sdp_HeaderB::get_rs() const
{
    return rs;
}

void Sdp_HeaderB::set_rs(const unsigned int &_rs)
{
    _string_representation_up2date = false;
    rs = _rs;
}

unsigned int Sdp_HeaderB::get_rr() const
{
    return rr;
}

void Sdp_HeaderB::set_rr(const unsigned int &_rr)
{
    _string_representation_up2date = false;
    rs = _rr;
}

std::string Sdp_HeaderB::get_string() const
{
    if (_string_representation_up2date)
        return _string_representation;

    std::string result;
    char num[30];
    if(ct != 0)
    {
        result += "CT:";
        sprintf(num, "%u", ct);
    }
    else if(as != 0)
    {
        result += "AS:";
        sprintf(num, "%u", as);
    }
    else if(tias != 0)
    {
        result += "TIAS:";
        sprintf(num, "%u", tias);
    }
    else if(rs != 0)
    {
        result += "RS:";
        sprintf(num, "%u", as);
    }
    else if(rr != 0)
    {
        result += "RR:";
        sprintf(num, "%u", as);
    }
    if(!result.empty())
        result = std::string("b=") + result + num;

    return result;
}
