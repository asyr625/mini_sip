#include "rtcp_mgr.h"

#include "rtcp_report_fir.h"
#include "rtcp_report_sdes.h"
#include "rtcp_report_sr.h"
#include "rtcp_report_reception_block.h"
#include "rtcp_report_app_view.h"
#include "rtcp_report_app_camctrl.h"

#include "sdes_cname.h"
#include "sdes_loc.h"
#include "session.h"
#include "my_time.h"
#include "video_codec.h"
#include "streams_player.h"

#include "session.h"
#include "mini_sip.h"
#include "sip_configuration.h"

#include "visca_ctrl.h"

#include <iostream>
#include <vector>
#include <climits>

using namespace std;

std::list<SRef<Rtcp_Callback*> > Rtcp_Mgr::rtcp_callbacks_global;

Rtcp_Mgr::Rtcp_Mgr(const std::string &_callID, IStreams_Player_Report_Timestamps *_streamsPlayer, Session* s)
    : call_id(_callID), streams_player(_streamsPlayer), session(s), nrtp_rcv(0), nrtp_send(0),
      bytes_rtp_send(0), time_last_rtcp_sent(0), rtcp_interval_ms(5000), ssrc(0)
{
    visca = NULL;
    local_sdes_location_str = session->get_mini_sip()->get_config()->_sdes_location;
    local_sdes_cname_str = session->get_own_identity()->get_sip_uri().get_user_name()+"@"+session->get_local_ip();
}

Rtcp_Mgr::~Rtcp_Mgr()
{
}

void Rtcp_Mgr::rtp_received_pre(SRef<SRtp_Packet *>&)
{
    ++nrtp_rcv;
}

SRef<Rtcp_Packet*> Rtcp_Mgr::rtp_received_post(SRef<SRtp_Packet *>& p)
{
    if (!p)
        return NULL;

    uint32_t ssrc=p->get_header().get_ssrc();

    map<uint32_t,struct rstat>::iterator it = ssrc_rstat.find(ssrc);

    if( it == ssrc_rstat.end() )
    {
        struct rstat emptyrstat = {0};
        ssrc_rstat[ssrc] = emptyrstat;
        it = ssrc_rstat.find(ssrc);
    }

    (*it).second.nrcv++;
    (*it).second.bytesrcv += p->get_content_length();
    uint16_t rtpSeq = p->get_header().get_seq_no();
    if ( rtpSeq > (*it).second.rtpSeqMax )
    {
        (*it).second.rtpSeqMax = rtpSeq;
    }

    send_queue_lock.lock();
    if (!send_queue.empty()) //if there are packets to be sent out, return it before testing for expired timers.
    {
        SRef<Rtcp_Packet*> ret = send_queue.front();
        send_queue.pop();
        send_queue_lock.unlock();
        return ret;
    }
    send_queue_lock.unlock();

    return NULL;
}

int albertohack_width = -1;
int albertohack_height= -1;

SRef<Rtcp_Packet*> Rtcp_Mgr::rtcp_received(SRef<Rtcp_Packet*>& rtcp)
{
    bool handled = false;

    if(streams_player)
    {
        std::vector<Rtcp_Report *> &reports = rtcp->get_reports();
        for(std::vector<Rtcp_Report *>::iterator i = reports.begin(); i != reports.end(); ++i)
        {
            Rtcp_Report_SR *senderReport = dynamic_cast<Rtcp_Report_SR *>(*i);
            if(senderReport)
            {
                //FIXME: re-enable handling SR reports after fixing stability problem
#if 0
                Rtcp_ReportSenderInfo *senderInfo = *senderReport->get_sender_info();
                if(senderInfo) {
                    streamsPlayer->add_rtcp_sender_report_timestamps(callID,
                                                                 senderReport->get_sender_ssrc(),
                                                                 senderInfo->get_ntp_timestamp_usecs_since_jan1st1970(),
                                                                 senderInfo->get_rtp_timestamp());
                    handled = true;
                    break;
                }
#endif
            }

            Rtcp_Report_Sdes *sdesReport = dynamic_cast<Rtcp_Report_Sdes *>(*i);
            if (sdesReport)
            {
                uint32_t ssrc;
                Sdes_Item* si = sdesReport->get_item(SDES_ITEM_LOC, &ssrc);
                if (si)
                {
                    Sdes_Loc* l = dynamic_cast<Sdes_Loc*>(si);
                    my_assert(l);
                    string_lock.lock();
                    ssrc_remote_sdes_location_str[ssrc]=l->get_string();
                    string_lock.unlock();
                }
                si = sdesReport->get_item(SDES_ITEM_CNAME, &ssrc);
                if (si)
                {
                    Sdes_Name* cn = dynamic_cast<Sdes_Name*>(si);
                    my_assert(cn);
                    string_lock.lock();
                    ssrc_remote_sdes_cname_str[ssrc]=cn->get_string();
                    string_lock.unlock();
                }
                handled = true;
                break;
            }


            Rtcp_Report_Fir *fir= dynamic_cast<Rtcp_Report_Fir*>(*i);
            if(fir) {
                uint32_t ssrc = fir->get_ssrc();
                SRef<Realtime_Media_Stream_Sender*> rtmss = session->get_realtime_media_stream_sender(ssrc);
                if (rtmss)
                {
                    rtmss->request_codec_intracoded();
                }
                handled = true;
                break;
            }



#ifdef VISCA_CAMCTRL
            Rtcp_Report_App_Camctrl *cctrl= dynamic_cast<Rtcp_Report_App_Camctrl*>(*i);
            if(cctrl)
            {
                if (!visca)
                {
                    visca =new Visca_Ctrl("/dev/ttyUSB0");
                }

                //cerr << "VISCA: "<< cctrl->getHSpeed() <<" "<< cctrl->getVSpeed() <<" "<< cctrl->getZSpeed()<<" "<< cctrl->getDuration()<<endl;
                visca->set_pan_tilt_zoom( cctrl->get_hspeed(), cctrl->get_vspeed(), cctrl->get_zspeed(), cctrl->get_duration() );
                //cerr <<"VISCA: done sending command"<<endl;

                handled=true;
                break;
            }
#endif
            Rtcp_Report_App_View *view= dynamic_cast<Rtcp_Report_App_View*>(*i);
            if(view) {

                int width = view->get_sender_width();
                int height = view->get_sender_height();
                unsigned ssrc = view->get_sender_ssrc();
                //if (width<320)
                //	width=320;
                //if (height<200)
                //	height=200;

#if 0
                albertohack_width = width;
                albertohack_height = height;

#endif
                cerr <<"**** AAAA DEBUG: RTCP AppView packet handled **** for: "<< ssrc <<" WxH: " <<width <<"x" <<height <<endl;

                SRef<Realtime_Media_Stream_Sender*> sender = session->get_realtime_media_stream_sender(ssrc);
                if(sender)
                {
                    SRef<Encoder_Instance*> encoder = sender->get_selected_encoder();
                    if(encoder)
                    {
                        if (dynamic_cast<Video_Encoder_Instance*>(*encoder))
                        {
                            Video_Encoder_Instance* vencoder = dynamic_cast<Video_Encoder_Instance*>(*encoder);
                            /*Sanity check that we keep ratio*/
                            if(width % 2 != 0) width++;
                            if(height % 2 != 0) height++;
                            if(height!=0 && vencoder->get_video_ratio()==(width/height) )
                            {
                                if(vencoder->set_video_size(width, height)<0)
                                    cerr << "RTCPMgr: wrong video ratio. We do not apply change" <<endl;
                            }else{
                                cerr << "RTCPMgr: wrong video ratio. We do not apply change" <<endl;
                            }
                        }
                    }
                }else{
                    cerr<< "RTCPMgr SSRC not found"<<endl;
                }
                //scalingplugin->setResize(width,height);

                handled = true;
                break;
            }
        }
    }

    if (!handled)
    {
        list<SRef<Rtcp_Callback*> >::iterator i;
        for (i=rtcp_callbacks.begin(); i != rtcp_callbacks.end(); i++)
            if ( (*i)->handle_rtcp(rtcp) )
                return NULL;

        for (i=rtcp_callbacks_global.begin(); i!=rtcp_callbacks_global.end(); i++)
            if ( (*i)->handle_rtcp(rtcp) )
                return NULL;
    }

    if(!handled)
        cerr <<"RtcpMgr::rtcpReceived: warning: RTCP ignored"<<endl;
    return NULL;
}

SRef<Rtcp_Packet*> Rtcp_Mgr::timeout()
{
    send_queue_lock.lock();
    if (!send_queue.empty())  //if there are packets to be sent out, return it before testing for expired timers.
    {
        SRef<Rtcp_Packet*> ret = send_queue.front();
        send_queue.pop();
        send_queue_lock.unlock();
        return ret;
    }
    send_queue_lock.unlock();
    return NULL;
}

SRef<Rtcp_Packet*> Rtcp_Mgr::rtp_sent(Rtp_Packet*p)
{
    ++nrtp_send;
    bytes_rtp_send += p->size();

    if (ssrc == 0)
        ssrc = p->get_header().get_ssrc();

    send_queue_lock.lock();
    if (!send_queue.empty()) //if there are packets to be sent out, return it before testing for expired timers.
    {
        SRef<Rtcp_Packet*> ret = send_queue.front();
        send_queue.pop();
        send_queue_lock.unlock();
        return ret;
    }
    send_queue_lock.unlock();
    return create_rtcp(true, p->get_header().timestamp);
}

SRef<Rtcp_Packet*> Rtcp_Mgr::rtp_sent(SRef<Rtp_Packet*> p)
{
    return rtp_sent(*p);
}

void Rtcp_Mgr::send_rtcp( SRef<Rtcp_Packet*> pkt)
{
    send_queue_lock.lock();
    send_queue.push(pkt);
    send_queue_lock.unlock();
}

std::string Rtcp_Mgr::get_remote_sdes_cname(uint32_t ssrc)
{
    string ret;
    string_lock.lock();
    ret = ssrc_remote_sdes_cname_str[ssrc];
    string_lock.unlock();
    return ret;
}

std::string Rtcp_Mgr::get_remote_sdes_loc(uint32_t ssrc)
{
    string ret;
    string_lock.lock();
    ret = ssrc_remote_sdes_location_str[ssrc];
    string_lock.unlock();
    return ret;
}

bool Rtcp_Mgr::has_callback(const SRef<Rtcp_Callback*>& cb)
{
    list<SRef<Rtcp_Callback*> >::iterator i;
    for (i=rtcp_callbacks.begin(); i!=rtcp_callbacks.end(); i++)
        if ( *cb==*(*i) )
            return true;
    return false;
}

void Rtcp_Mgr::add_rtcp_callback(const SRef<Rtcp_Callback*>& cb)
{
    rtcp_callbacks.push_back(cb);
}

void Rtcp_Mgr::remove_rtcp_callback(const SRef<Rtcp_Callback*>& cb)
{
    rtcp_callbacks.remove(cb);
}

bool Rtcp_Mgr::has_callback_global(const SRef<Rtcp_Callback*>& cb)
{
    list<SRef<Rtcp_Callback*> >::iterator i;
    for (i=rtcp_callbacks_global.begin(); i!=rtcp_callbacks_global.end(); i++)
        if ( *cb==*(*i) )
            return true;
    return false;
}

void Rtcp_Mgr::add_rtcp_callback_global(const SRef<Rtcp_Callback*>& cb)
{
    rtcp_callbacks_global.push_back(cb);
}

void Rtcp_Mgr::remove_rtcp_callback_global(const SRef<Rtcp_Callback*>& cb)
{
    rtcp_callbacks_global.remove(cb);
}

bool Rtcp_Mgr::rtcp_interval_expired()
{
    uint64_t now = my_time();
    if(time_last_rtcp_sent == 0)
        time_last_rtcp_sent = now;
    bool ret = now > time_last_rtcp_sent + rtcp_interval_ms;
    if(ret)
        time_last_rtcp_sent = now;
    return ret;
}

SRef<Rtcp_Packet*> Rtcp_Mgr::create_rtcp(const bool &includeSenderReport, const uint32_t &senderRtpPts)
{
    SRef<Rtcp_Packet*> p = new Rtcp_Packet;

    if (ssrc && includeSenderReport && rtcp_interval_expired())
    {
        Rtcp_Report_SR* sr = new Rtcp_Report_SR(ssrc);

        Rtcp_Report_Sender_Info info;
        long double now = utime(true) / (long double)1000000; //time in s since epoch
        now += ((long double)25567) * 24 * 60 * 60; //2272060800ULL; //time in s since 0h Jan 1, 1900
        info.set_ntp_timestamp(now);
        info.set_rtp_timestamp(senderRtpPts);
        info.set_sender_packet_count( nrtp_send );
        info.set_sender_octet_count( bytes_rtp_send );
        sr->set_sender_info(info);

        /*
         * better not to place the receiver's report blocks at all than to mislead other parties
         map<uint32_t, struct rstat>::const_iterator i;
         for (i=ssrc_rstat.begin(); i!=ssrc_rstat.end(); i++) {
         Rtcp_ReportReceptionBlock rb( (*i).first );
         rb.set_fraction_lost(33); rb.set_cumulative_n_lost(33); rb.set_seq_high(33); rb.set_jitter(33); rb.set_last_sr(33); rb.set_dlsr(33);
         sr->add_reception_block(rb);
         }
         */
        p->add_report( sr );
        Sdes_Cname *cname = new Sdes_Cname(local_sdes_cname_str);
        Sdes_Chunk* sdeschunk = new Sdes_Chunk(ssrc, cname);

        if (local_sdes_location_str.size()>0)
        {
            SRef<Message_Router*> mr = session->get_message_router();

            string muteString;

            try{
                Command_String req("local","get_mute_string"/*ignored*/);
                Command_String resp = mr->handle_command_resp("plugin_volumeinfo",req);
                muteString = resp.get_param();
            }catch(Subsystem_Not_Found_Exception&)
            { }


            string timeString;
            uint64_t t = 0;
            try{
                Command_String req("local","get_face_count"/*ignored*/);
                Command_String resp = mr->handle_command_resp("plugin_facedetect",req);
                t = atoi(resp.get_param().c_str());
                t=t/1000;
                if (t<10)
                    timeString=", last seen: now";
                else if (t<120)
                    timeString = ", last seen "+itoa(t)+" seconds ago";
                else{
                    t=t/60;
                    if (t<120){
                        timeString= ", last seen "+itoa(t)+" minutes ago";
                    }else{

                        t=t/60;
                        if(t<48)
                            timeString = ", last seen "+itoa(t)+" hours ago";
                        else{
                            t=t/24;
                            timeString = ", last seen "+itoa(t)+" days ago";
                            if(t>14)
                                timeString = "";
                        }
                    }
                }

            }catch(Subsystem_Not_Found_Exception&){
                //cerr<<"EEEEEEEEE: subsystem not found!"<<endl;
            }

            string locStr = local_sdes_location_str + timeString;
            if (muteString.size()>0)
                locStr=locStr+", ("+muteString+")";
            Sdes_Loc *loc = new Sdes_Loc(locStr);
            sdeschunk->add_item(loc);
        }

        Rtcp_Report_Sdes* sdes = new Rtcp_Report_Sdes(sdeschunk);
        p->add_report(sdes);
        return p;
    }
    return NULL;
}
