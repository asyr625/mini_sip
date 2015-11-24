#ifndef RTP_STREAM_H
#define RTP_STREAM_H

#include "sobject.h"

#include "rtcp_mgr.h"
#include "udp_socket.h"

class IStreams_Player_Report_Timestamps;

class Rtp_Stream : public SObject
{
public:
    static int rtp_port_range_start;
    static int rtp_port_range_end;

    Rtp_Stream(bool useIpv6, const std::string &_callID, IStreams_Player_Report_Timestamps *_streamsPlayer, Session* session);
    virtual ~Rtp_Stream();

    void send(const SRef<Rtp_Packet *> &packet, const IPAddress &to_addr, const uint16_t &port);
    SRef<UDP_Socket*> get_rtp_socket();
    SRef<UDP_Socket*> get_rtcp_socket();
    SRef<Rtcp_Mgr*> get_rtcp_mgr();

private:
    SRef<UDP_Socket*> rtp_socket;
    SRef<UDP_Socket*> rtcp_socket;
    SRef<Rtcp_Mgr*> rtcp_mgr;
};

#endif // RTP_STREAM_H
