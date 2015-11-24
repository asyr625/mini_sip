#ifndef SIP_TRANSITION_UTILS_H
#define SIP_TRANSITION_UTILS_H

#include "sobject.h"
#include "sip_smcommand.h"

class Sip_Response;


bool sip_response_filter_match(SRef<Sip_Response*> resp, const std::string &pattern);

#define IGN -1

bool transition_match( const std::string& packetType, const Sip_SMCommand &command, int source, int destination,
        const std::string &respFilter="");

/** Match Sip responses */
bool transition_match_sip_response( const std::string& cseqMethod, const Sip_SMCommand &command, int source, int destination,
        const std::string &respFilter="");

bool transition_match( const Sip_SMCommand &command, const std::string &cmd_str, int source, int destination);

#endif // SIP_TRANSITION_UTILS_H
