#include "sip_header_subscription_state.h"

SRef<Sip_Header_Value *> subscriptionStateFactory(const std::string &build_from)
{
    return new Sip_Header_Value_Subscription_State(build_from);
}

Sip_Header_Factory_Func_Ptr sip_header_subscription_state_factory = subscriptionStateFactory;


const std::string sipHeaderValueSubscriptionStateTypeStr = "Subscription-State";

Sip_Header_Value_Subscription_State::Sip_Header_Value_Subscription_State(const std::string &build_from)
    : Sip_Header_Value_String(SIP_HEADER_TYPE_SUBSCRIPTIONSTATE, sipHeaderValueSubscriptionStateTypeStr,build_from)
{
}
