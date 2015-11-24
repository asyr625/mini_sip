#ifndef SIP_HEADER_CSEQ_H
#define SIP_HEADER_CSEQ_H

#include "sip_header.h"

extern Sip_Header_Factory_Func_Ptr sip_header_cseq_factory;

class Sip_Header_Value_CSeq : public Sip_Header_Value
{
public:
    Sip_Header_Value_CSeq(const std::string &method, int seq);
    Sip_Header_Value_CSeq(const std::string &build_from);

    virtual ~Sip_Header_Value_CSeq();

    virtual std::string get_mem_object_type() const { return "SipHeaderCSeq"; }

    std::string get_string() const;
    int32_t get_cseq() const;
    void set_cseq(int32_t n);

    std::string get_method() const;
    void set_method(const std::string &method);

private:
    std::string _method;
    int32_t _seq;
};

#endif // SIP_HEADER_CSEQ_H
