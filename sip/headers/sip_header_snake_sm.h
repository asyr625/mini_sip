#ifndef SIP_HEADER_SNAKE_SM_H
#define SIP_HEADER_SNAKE_SM_H

#include "sip_header_string.h"

extern Sip_Header_Factory_Func_Ptr sip_header_snake_sm_factory;

class Sip_Header_Value_Snake_SM : public Sip_Header_Value_String
{
public:
    Sip_Header_Value_Snake_SM(const std::string &build_from);

    virtual std::string get_mem_object_type() const { return "SipHeaderSnakeSM"; }
};

#endif // SIP_HEADER_SNAKE_SM_H
