#include "sip_header_content_type.h"


SRef<Sip_Header_Value *> contenttypeFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Content_Type(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_content_type_factory = contenttypeFactory;

const std::string sipHeaderValueContentTypeTypeStr = "Content-Type";

Sip_Header_Value_Content_Type::Sip_Header_Value_Content_Type(const std::string &build_from)
    : Sip_Header_Value_String(SIP_HEADER_TYPE_CONTENTTYPE,sipHeaderValueContentTypeTypeStr, build_from)
{
}
