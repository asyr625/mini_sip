#ifndef SDP_PACKET_H
#define SDP_PACKET_H

#include <vector>
#include "sobject.h"
#include "ipaddress.h"
#include "sip_message_content.h"
#include "sip_message_content_factory.h"

#include "sdp_header.h"
#include "sdp_headerm.h"
#include "sdp_headerc.h"

SRef<Sip_Message_Content*> sdpSipMessageContentFactory(const std::string & buf, const std::string & ContentType);

class Sdp_Packet : public Sip_Message_Content
{
public:
    Sdp_Packet();
    Sdp_Packet(std::string build_from);

    virtual std::string get_mem_object_type() const {return "SdpPacket";}

    SRef<Sdp_HeaderC*> get_session_level_connection();
    std::string get_key_mgmt();
    void add_header(SRef<Sdp_Header*> h);
    virtual std::string get_string() const;
    virtual std::string get_content_type() const{return "application/sdp";}


    std::vector<SRef<Sdp_Header*> > get_headers();
    std::string get_format_match(Sdp_Packet &pack);
    std::string get_first_media_format();
    bool media_format_available(std::string f);

    void set_session_level_attribute(std::string type, std::string value);

    std::string get_session_level_attribute(std::string type);

    void remove_last_header();
private:
    std::vector<SRef<Sdp_Header*> > headers;
};

#endif // SDP_PACKET_H
