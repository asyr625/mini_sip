#ifndef SIP_HEADER_RSEQ_H
#define SIP_HEADER_RSEQ_H

#include "sip_header.h"

extern Sip_Header_Factory_Func_Ptr sip_header_rseq_factory;

class Sip_Header_Value_RSeq : public Sip_Header_Value
{
public:
    Sip_Header_Value_RSeq(uint32_t rnum);
    Sip_Header_Value_RSeq(const std::string &build_from);

    virtual ~Sip_Header_Value_RSeq();

    virtual std::string get_mem_object_type() const {return "SipHeaderRSeq";}

    virtual std::string get_string() const;

    uint32_t get_rseq() const;
    void set_rseq( uint32_t rseq );


private:
    uint32_t _seq;
};

#endif // SIP_HEADER_RSEQ_H
