#include "sip_header_session_expires.h"
#include "string_utils.h"

SRef<Sip_Header_Value *> sessionExpiresFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Session_Expires(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_session_expires_factory = sessionExpiresFactory;

const std::string sipHeaderValueSessionExpires = "Session-Expires";

Sip_Header_Value_Session_Expires::Sip_Header_Value_Session_Expires(const std::string &build_from)
    : Sip_Header_Value(SIP_HEADER_TYPE_FROM,sipHeaderValueSessionExpires),
      timeout(0)
{
    std::string timeoutstr;
    int i=0;
    int len = build_from.size();
    //skip starting whitespace
    while (i<len && (build_from[i]==' ' || build_from[i]=='\r' || build_from[i]=='\n') )
    {
        i++;
    }

    while (i<len && build_from[i]>=0 && build_from[i]<=9)
        timeoutstr = timeoutstr + build_from[i];

    int t = atoi( timeoutstr.c_str() );
    if (t<0)
        return;
    timeout = t;
}

Sip_Header_Value_Session_Expires::~Sip_Header_Value_Session_Expires()
{
}

std::string Sip_Header_Value_Session_Expires::get_string() const
{
    return itoa(timeout);
}

uint32_t Sip_Header_Value_Session_Expires::get_timeout() const
{
    return timeout;
}
