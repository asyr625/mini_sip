#include "sip_header_cseq.h"
#include "string_utils.h"

SRef<Sip_Header_Value *> cseqFactory(const std::string &build_from){
    return new Sip_Header_Value_CSeq(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_cseq_factory = cseqFactory;

const std::string sipHeaderValueCSeqTypeStr = "CSeq";

Sip_Header_Value_CSeq::Sip_Header_Value_CSeq(const std::string &build_from)
    : Sip_Header_Value(SIP_HEADER_TYPE_CSEQ,sipHeaderValueCSeqTypeStr)
{
    unsigned maxlen=(unsigned)build_from.size();
    unsigned i=0;

    while (i<maxlen && is_ws(build_from[i]))
        i++;

    std::string num;
    while (i<maxlen && ((build_from[i]>='0' && build_from[i]<='9') || build_from[i]=='-'))
    {
        num+=build_from[i];
        i++;
    }

    while (i<maxlen && is_ws(build_from[i]) )
        i++;

    _method = build_from.substr(i);

    set_cseq(atoi((trim(num)).c_str()));
}


Sip_Header_Value_CSeq::Sip_Header_Value_CSeq(const std::string &meth, int s)
    : Sip_Header_Value(SIP_HEADER_TYPE_CSEQ,sipHeaderValueCSeqTypeStr),
      _method(meth), _seq(s)
{
}

Sip_Header_Value_CSeq::~Sip_Header_Value_CSeq()
{
}

std::string Sip_Header_Value_CSeq::get_string() const
{
    return itoa(_seq) +" "+ _method;
}

std::string Sip_Header_Value_CSeq::get_method() const
{
    return _method;
}

void Sip_Header_Value_CSeq::set_method(const std::string &m)
{
    this->_method = m;
}

void Sip_Header_Value_CSeq::set_cseq(int32_t n)
{
    this->_seq = n;
}

int32_t Sip_Header_Value_CSeq::get_cseq() const
{
    return _seq;
}
