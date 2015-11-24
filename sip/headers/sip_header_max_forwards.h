#ifndef SIP_HEADER_VALUE_MAX_FORWARDS_H
#define SIP_HEADER_VALUE_MAX_FORWARDS_H

#include "sip_header.h"

extern Sip_Header_Factory_Func_Ptr sip_header_max_forwards_factory;

class Sip_Header_Value_Max_Forwards : public Sip_Header_Value
{
public:
    Sip_Header_Value_Max_Forwards(int32_t mf);
    Sip_Header_Value_Max_Forwards(const std::string &build_from);

    virtual ~Sip_Header_Value_Max_Forwards();

    virtual std::string get_mem_object_type() const {return "SipHeaderMaxForwards";}
    std::string get_string() const;
    int32_t get_max_forwards() const;

    void set_max_forwards(int32_t max);

private:
    int32_t _max;
};

#endif // SIP_HEADER_VALUE_MAX_FORWARDS_H
