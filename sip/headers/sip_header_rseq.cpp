#include "sip_header_rseq.h"
#include "sip_exception.h"
#include "string_utils.h"
SRef<Sip_Header_Value *> rseqFactory(const std::string &build_from)
{
    return new Sip_Header_Value_RSeq(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_rseq_factory = rseqFactory;

const std::string Sip_Header_Value_RSeqTypeStr = "RSeq";

Sip_Header_Value_RSeq::Sip_Header_Value_RSeq(const std::string &build_from)
    : Sip_Header_Value(SIP_HEADER_TYPE_RSEQ,Sip_Header_Value_RSeqTypeStr)
{
    unsigned i=0;
    unsigned len=(unsigned)build_from.size();
    while (is_ws(build_from[i]))
        i++;
    std::string num="";
    while (i<len && build_from[i]>='0' && build_from[i]<='9')
    {
        num+=build_from[i];
        i++;
    }

    while (i<len && is_ws(build_from[i]))
        i++;

    if (num.size()==0 || i<len )
    {
        throw Sip_Exception_Invalid_Message("Sip_Header_Value_RSeq malformed");
    }
#ifdef _MSC_VER
    _seq = (uint32_t)_atoi64(num.c_str());
#else
    _seq = atoll(num.c_str());
#endif
}

Sip_Header_Value_RSeq::Sip_Header_Value_RSeq(uint32_t n)
    : Sip_Header_Value(SIP_HEADER_TYPE_RSEQ,Sip_Header_Value_RSeqTypeStr)
{
    _seq = n;
}

Sip_Header_Value_RSeq::~Sip_Header_Value_RSeq()
{
}

std::string Sip_Header_Value_RSeq::get_string() const
{
    return itoa(_seq);
}

uint32_t Sip_Header_Value_RSeq::get_rseq() const
{
    return _seq;
}

void Sip_Header_Value_RSeq::set_rseq( uint32_t rseq )
{
    _seq = rseq;
}
