#ifndef SIP_HEADER_RACK_H
#define SIP_HEADER_RACK_H

#include "sip_header.h"

extern Sip_Header_Factory_Func_Ptr sip_header_rack_factory;

class Sip_Header_Value_RAck : public Sip_Header_Value
{
public:
    Sip_Header_Value_RAck(const std::string &method, int responseNum, int cseqNum);
    Sip_Header_Value_RAck(const std::string &build_from);

    virtual ~Sip_Header_Value_RAck();

    virtual std::string get_mem_object_type() const {return "SipHeaderRAck";}

    std::string get_string() const;

    int32_t get_response_num() const;

    int32_t get_cseq_num() const;


    std::string get_method() const;
    void set_method(const std::string &method);

private:
    std::string _method;
    int32_t _cseqnum;
    int32_t _respnum;
};

#endif // SIP_HEADER_RACK_H
