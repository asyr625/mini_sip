#include "sip_transition_utils.h"
#include "sip_response.h"
#include "string_utils.h"

using namespace std;
bool sip_response_filter_match(SRef<Sip_Response*> resp, const std::string &pattern)
{
    int32_t status = resp->get_status_code();

    if ( (pattern[0]=='*' || (status/100==(pattern[0]-'0'))) && (pattern[1]=='*' || ((status/10)%10 == pattern[1]-'0')) &&
            (pattern[2]=='*' || (status%10 == pattern[2]-'0') ))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool transition_match( const std::string& packetType, const Sip_SMCommand &command, int source, int destination,
        const std::string &respFilter)
{
    if (source!=IGN && command.get_source() != source)
    {
        return false;
    }
    if ( destination!=IGN && command.get_destination() != destination)
    {
        return false;
    }
    if (command.get_type()!=Sip_SMCommand::COMMAND_PACKET)
    {
        return false;
    }
    if (packetType!="" && command.get_command_packet()->get_type()!=packetType)
    {
        return false;
    }
    if (respFilter.size()>0)
    {
        std::vector<string> filters = split_lines(respFilter);
        for (vector<string>::iterator i = filters.begin(); i != filters.end(); i++)
        {
            if (sip_response_filter_match( SRef<Sip_Response*> ( (Sip_Response *)*command.get_command_packet() ), *i ))
            {
                return true;
            }
        }
        return false;
    }
    return true;
}

bool transition_match_sip_response( const std::string& cseqMethod, const Sip_SMCommand &command, int source, int destination,
        const std::string &respFilter)
{
    if( !transition_match( Sip_Response::type, command, source, destination, respFilter ) )
        return false;

    return cseqMethod=="*" || command.get_command_packet()->get_cseq_method() == cseqMethod;
}

bool transition_match( const Sip_SMCommand &command, const std::string &cmd_str, int source, int destination)
{
    if (command.get_type()!=Sip_SMCommand::COMMAND_STRING)
    {
        return false;
    }
    if ( destination!=IGN && command.get_destination() != destination)
    {
        return false;
    }
    if ( source!=IGN && command.get_source() != source)
    {
        return false;
    }
    if (command.get_command_string().get_op()!=cmd_str)
    {
        return false;
    }
    return true;
}
