#ifndef SIP_HEADER_SUBSCRIPTION_STATE_H
#define SIP_HEADER_SUBSCRIPTION_STATE_H

#include "sip_header_string.h"

extern Sip_Header_Factory_Func_Ptr sip_header_subscription_state_factory;

class Sip_Header_Value_Subscription_State : public Sip_Header_Value_String
{
public:
    Sip_Header_Value_Subscription_State(const std::string &build_from);

    virtual std::string get_mem_object_type() const { return "SipHeaderSubscriptionState"; }
};

#endif // SIP_HEADER_SUBSCRIPTION_STATE_H
