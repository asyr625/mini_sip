#include "sip_header_to.h"


SRef<Sip_Header_Value *> toFactory(const std::string &build_from){
                    return new Sip_Header_Value_To(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_to_factory = toFactory;


const std::string Sip_Header_Value_ToTypeStr = "To";


Sip_Header_Value_To::Sip_Header_Value_To(const std::string &build_from)
        : Sip_Header_Value(SIP_HEADER_TYPE_TO,Sip_Header_Value_ToTypeStr),uri(build_from)
{
}


Sip_Header_Value_To::Sip_Header_Value_To(const Sip_Uri& u)
        : Sip_Header_Value(SIP_HEADER_TYPE_TO,Sip_Header_Value_ToTypeStr),
          uri(u)
{
}

Sip_Header_Value_To::~Sip_Header_Value_To()
{
}

std::string Sip_Header_Value_To::get_string() const
{
    return uri.get_string();
}

Sip_Uri Sip_Header_Value_To::get_uri() const
{
    return uri;
}

void Sip_Header_Value_To::set_uri(const Sip_Uri &u)
{
    this->uri=u;
}
