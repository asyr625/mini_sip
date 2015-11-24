#include "sip_header_refer_to.h"


SRef<Sip_Header_Value *> refertoFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Refer_To(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_refer_to_factory = refertoFactory;


const std::string sipHeaderValueReferToTypeStr = "Refer-To";

Sip_Header_Value_Refer_To::Sip_Header_Value_Refer_To(const std::string &build_from)
    : Sip_Header_Value_String(SIP_HEADER_TYPE_REFERTO,sipHeaderValueReferToTypeStr, build_from)
{
}
