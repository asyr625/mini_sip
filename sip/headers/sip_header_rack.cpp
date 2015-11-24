#include "sip_header_rack.h"
#include "string_utils.h"

SRef<Sip_Header_Value *> rackFactory(const std::string &build_from)
{
    return new Sip_Header_Value_RAck(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_rack_factory = rackFactory;

const std::string sipHeaderValueRAckTypeStr = "RAck";

Sip_Header_Value_RAck::Sip_Header_Value_RAck(const std::string &build_from)
    : Sip_Header_Value(SIP_HEADER_TYPE_RACK,sipHeaderValueRAckTypeStr)
{
    unsigned i=0;
    unsigned maxi = (unsigned)build_from.size()-1;
    while(i <= maxi && is_ws(build_from[i]) )
        i++;

    std::string respnumstr;
    while( i<= maxi && ( (build_from[i]>='0' && build_from[i]<='9') || build_from[i]=='-') )
    {
        respnumstr+=build_from[i];
        i++;
    }

    while (i<=maxi && is_ws(build_from[i]) )
        i++;

    std::string cseqnumstr;
    while (i<=maxi && ((build_from[i]>='0' && build_from[i]<='9') || build_from[i]=='-'))
    {
        cseqnumstr += build_from[i];
        i++;
    }

    while (i<=maxi && is_ws(build_from[i]) )
        i++;

    _method = build_from.substr(i);

    _respnum = atoi((trim(respnumstr)).c_str());
    _cseqnum = atoi((trim(cseqnumstr)).c_str());
}


Sip_Header_Value_RAck::Sip_Header_Value_RAck(const std::string &method, int responseNum, int cseqNum)
    : Sip_Header_Value(SIP_HEADER_TYPE_RACK,sipHeaderValueRAckTypeStr),
      _method(method),
      _cseqnum(responseNum),
      _respnum(cseqNum)
{

}

Sip_Header_Value_RAck::~Sip_Header_Value_RAck()
{
}


std::string Sip_Header_Value_RAck::get_string() const
{
    return itoa(_respnum) +" "+ itoa(_cseqnum) +" "+ _method;
}

int32_t Sip_Header_Value_RAck::get_response_num() const
{
    return _respnum;
}

int32_t Sip_Header_Value_RAck::get_cseq_num() const
{
    return _cseqnum;
}


std::string Sip_Header_Value_RAck::get_method() const
{
    return _method;
}
void Sip_Header_Value_RAck::set_method(const std::string &method)
{
    _method = method;
}
