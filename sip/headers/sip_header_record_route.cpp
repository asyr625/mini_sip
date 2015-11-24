#include "sip_header_record_route.h"


SRef<Sip_Header_Value *> recordrouteFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Record_Route(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_record_route_factory = recordrouteFactory;


// Ex: Record-Route: <sip:vatn@213.100.38.57;ftag=2064763305;lr>,<...>

const std::string sipHeaderValueRecordRouteTypeStr = "Record-Route";

Sip_Header_Value_Record_Route::Sip_Header_Value_Record_Route(const std::string &build_from)
    : Sip_Header_Value_String(SIP_HEADER_TYPE_RECORDROUTE,sipHeaderValueRecordRouteTypeStr, build_from)
{
}
