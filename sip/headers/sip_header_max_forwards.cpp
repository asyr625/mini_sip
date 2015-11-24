#include "sip_header_max_forwards.h"
#include "string_utils.h"

SRef<Sip_Header_Value *> maxforwardsFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Max_Forwards(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_max_forwards_factory = maxforwardsFactory;

const std::string sipHeaderValueMaxForwardsTypeStr = "Max-Forwards";

Sip_Header_Value_Max_Forwards::Sip_Header_Value_Max_Forwards(int32_t mf)
    : Sip_Header_Value(SIP_HEADER_TYPE_MAXFORWARDS,sipHeaderValueMaxForwardsTypeStr),
      _max(mf)
{
}

Sip_Header_Value_Max_Forwards::Sip_Header_Value_Max_Forwards(const std::string &build_from)
    : Sip_Header_Value(SIP_HEADER_TYPE_MAXFORWARDS,sipHeaderValueMaxForwardsTypeStr),
      _max(-1)
{
    _max = atoi( trim( build_from).c_str());
}

Sip_Header_Value_Max_Forwards::~Sip_Header_Value_Max_Forwards()
{
}


std::string Sip_Header_Value_Max_Forwards::get_string() const
{
    return itoa(_max);
}

int32_t Sip_Header_Value_Max_Forwards::get_max_forwards() const
{
    return _max;
}

void Sip_Header_Value_Max_Forwards::set_max_forwards(int32_t max)
{
    _max = max;
}
