#include "sip_header_route.h"
#include "string_utils.h"
SRef<Sip_Header_Value *> routeFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Route(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_route_factory = routeFactory;

const std::string sipHeaderValueRouteTypeStr = "Route";

Sip_Header_Value_Route::Sip_Header_Value_Route(std::list<std::string> &routeSet)
    : Sip_Header_Value(SIP_HEADER_TYPE_ROUTE,sipHeaderValueRouteTypeStr)
{
    std::list<std::string>::iterator iter;
    std::list<std::string>::iterator iter_end = routeSet.end();
    for ( iter = routeSet.begin(); iter != iter_end; ++iter)
    {
        #ifdef DEBUG_OUTPUT
        cerr << "SipHeaderRoute:: route " << *iter << endl;
        #endif
        if( iter != routeSet.begin())
            _route = _route + ',';
        _route = _route + (*iter);
    }
}

Sip_Header_Value_Route::Sip_Header_Value_Route(const std::string &build_from)
    : Sip_Header_Value(SIP_HEADER_TYPE_ROUTE,sipHeaderValueRouteTypeStr)
{
    _route = trim(build_from);
}

Sip_Header_Value_Route::~Sip_Header_Value_Route()
{
}

std::string Sip_Header_Value_Route::get_string() const
{
    return _route;
}

std::string Sip_Header_Value_Route::get_route() const
{
    return _route;
}
