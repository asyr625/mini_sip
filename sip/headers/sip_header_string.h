#ifndef SIP_HEADER_STRING_H
#define SIP_HEADER_STRING_H

#include "sip_header.h"

extern Sip_Header_Factory_Func_Ptr sip_header_subject_factory;

class Sip_Header_Value_String : public Sip_Header_Value
{
public:
    Sip_Header_Value_String(int type, const std::string& typeStr, const std::string& build_from);

    virtual ~Sip_Header_Value_String();

    std::string get_string() const;

    void set_string(const std::string &newStr);

protected:
    std::string _str;
};

#endif // SIP_HEADER_STRING_H
