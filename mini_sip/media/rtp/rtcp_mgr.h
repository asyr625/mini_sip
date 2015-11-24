#ifndef RTCP_MGR_H
#define RTCP_MGR_H

#include<map>
#include<list>
#include<queue>

#include "sobject.h"
#include "mutex.h"

#include "srtp_packet.h"
#include "rtcp_packet.h"

class IStreams_Player_Report_Timestamps;

struct rstat
{
    uint32_t nrcv;
    uint32_t bytesrcv;
    uint16_t rtpSeqMax;
    uint16_t rtpSeqRollover;
};

class Session;
class Visca_Ctrl;

class Rtcp_Callback : public virtual SObject
{
public:
    virtual bool handle_rtcp(const SRef<Rtcp_Packet*>& rtcp) = 0;
};

class Rtcp_Mgr : public SObject
{
public:
    Rtcp_Mgr(const std::string &_callID, IStreams_Player_Report_Timestamps *_streamsPlayer, Session* s);
    ~Rtcp_Mgr();

    void rtp_received_pre(SRef<SRtp_Packet *>&);

    SRef<Rtcp_Packet*> rtp_received_post(SRef<SRtp_Packet *>&p);

    SRef<Rtcp_Packet*> rtcp_received(SRef<Rtcp_Packet*>& rtcp);

    SRef<Rtcp_Packet*> timeout();

    SRef<Rtcp_Packet*> rtp_sent(Rtp_Packet*p);
    SRef<Rtcp_Packet*> rtp_sent(SRef<Rtp_Packet*> p);

    void send_rtcp( SRef<Rtcp_Packet*> pkt);

    std::string get_remote_sdes_cname(uint32_t ssrc);
    std::string get_remote_sdes_loc(uint32_t ssrc);

    bool has_callback(const SRef<Rtcp_Callback*>& cb);
    void add_rtcp_callback(const SRef<Rtcp_Callback*>& cb);
    void remove_rtcp_callback(const SRef<Rtcp_Callback*>& cb);

    static bool has_callback_global(const SRef<Rtcp_Callback*>& cb);
    static void add_rtcp_callback_global(const SRef<Rtcp_Callback*>& cb);
    static void remove_rtcp_callback_global(const SRef<Rtcp_Callback*>& cb);

private:
    Mutex string_lock;
    bool rtcp_interval_expired();
    std::list<SRef<Rtcp_Callback*> > rtcp_callbacks;
    static std::list<SRef<Rtcp_Callback*> > rtcp_callbacks_global;
    std::string local_sdes_location_str;
    std::string local_sdes_cname_str;
    std::map<uint32_t, std::string> ssrc_remote_sdes_location_str;
    std::map<uint32_t, std::string> ssrc_remote_sdes_cname_str;

    SRef<Rtcp_Packet*> create_rtcp(const bool &includeSenderReport, const uint32_t &senderRtpPts=0);

    std::string call_id;
    IStreams_Player_Report_Timestamps *streams_player;
    Session* session;
    Visca_Ctrl *visca;
    uint64_t nrtp_rcv;
    uint64_t nrtp_send;
    uint64_t bytes_rtp_send;
    uint64_t time_last_rtcp_sent;
    uint32_t rtcp_interval_ms;
    uint32_t ssrc;

    std::map<uint32_t, struct rstat> ssrc_rstat;

    std::queue< SRef<Rtcp_Packet*> > send_queue;
    Mutex send_queue_lock;
};

#endif // RTCP_MGR_H
