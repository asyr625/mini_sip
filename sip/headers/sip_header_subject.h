#ifndef SIP_HEADER_SUBJECT_H
#define SIP_HEADER_SUBJECT_H
#include "sip_header_string.h"

extern Sip_Header_Factory_Func_Ptr sip_header_subject_factory;

class Sip_Header_Value_Subject : public Sip_Header_Value_String
{
public:
    Sip_Header_Value_Subject(const std::string &build_from);

    virtual std::string get_mem_object_type() const { return "SipHeaderSubject"; }
};

#endif // SIP_HEADER_SUBJECT_H
