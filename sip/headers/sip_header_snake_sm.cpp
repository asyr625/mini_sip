#include "sip_header_snake_sm.h"

SRef<Sip_Header_Value *> snakeSMFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Snake_SM(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_snake_sm_factory = snakeSMFactory;

const std::string sipHeaderValueSnakeSMTypeStr = "Snake-SM";

Sip_Header_Value_Snake_SM::Sip_Header_Value_Snake_SM(const std::string &build_from)
    : Sip_Header_Value_String(SIP_HEADER_TYPE_SNAKESM, sipHeaderValueSnakeSMTypeStr, build_from)
{
}
