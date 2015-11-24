#include "sip_header_accept.h"

SRef<Sip_Header_Value *> accept_factory(const std::string &build_from)
{
    return new Sip_Header_Value_Accept(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_accept_factory = accept_factory;

const std::string sipHeaderValueAcceptTypeStr="Accept";

Sip_Header_Value_Accept::Sip_Header_Value_Accept(const std::string &build_from)
    :Sip_Header_Value_String(SIP_HEADER_TYPE_ACCEPT, sipHeaderValueAcceptTypeStr, build_from)
{
}
