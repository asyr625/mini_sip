#include "rtp_receiver.h"
#include <string>

#include "udp_socket.h"
#include "network_exception.h"
#include "thread.h"

#include "srtp_packet.h"
#include "rtcp_packet.h"
#include "rtcp_mgr.h"

#include "codec.h"
#include "media_stream.h"
#include "ip_provider.h"

#include<iostream>

using namespace std;

#ifdef LOGGING_SUPPORT
#include "logging_manager.h"
#endif

#include<stdio.h>
#include<sys/types.h>
#include<stdlib.h> //for rand

#ifdef WIN32
#include<winsock2.h>
#endif

#ifdef _MSC_VER

#else
#include<sys/time.h>
#include<unistd.h>
#include<errno.h>
#endif

#ifdef _WIN32_WCE
#	include"../include/minisip_wce_extra_includes.h"
#endif


Rtp_Receiver::Rtp_Receiver( SRef<Ip_Provider *> ipProvider, std::string callId,
                            IStreams_Player_Report_Timestamps *_streamsPlayer, Session* session)
    : call_id(callId)
{
    std::string externalIp = ipProvider->get_external_ip();
    bool useIPv6;

    if( externalIp.find(':') == string::npos )
        useIPv6 = false;
    else
        useIPv6 = true;
    rtp_stream = new Rtp_Stream(useIPv6, callId, _streamsPlayer, session);
    external_port = ipProvider->get_external_port( rtp_stream->get_rtp_socket() );
    external_rtcp_port = ipProvider->get_external_port( rtp_stream->get_rtcp_socket() );

    kill = false;

    thread = new Thread(this, Thread::High_Priority);
}

Rtp_Receiver::~Rtp_Receiver()
{
    stop();
    join();
}

void Rtp_Receiver::register_realtime_media_stream( SRef<Realtime_Media_Stream_Receiver *> realtimeMediaStream )
{
    std::list< SRef<Realtime_Media_Stream_Receiver *> >::iterator iter;
    realtime_media_streams_lock.lock();
    /* Don't register new streams if the receiver is being closed */
    if( !kill )
    {
        bool found = false;
        for( iter = realtime_media_streams.begin(); iter != realtime_media_streams.end(); iter++ )
        {
            if( (*iter)->get_id() == realtimeMediaStream->get_id() )
            {
                found = true;
#ifdef DEBUG_OUTPUT
                cerr << "Rtp_Receiver::register_realtime_media_stream: media stream already registered. Updating SRef." << endl;
#endif
                (*iter) = realtimeMediaStream;
                break;
            }
        }
        if( !found )
        {
            realtime_media_streams.push_back( realtimeMediaStream );
        }
    }
    realtime_media_streams_lock.unlock();
}

void Rtp_Receiver::unregister_realtime_media_stream( SRef<Realtime_Media_Stream_Receiver *> realtimeMediaStream)
{
    my_assert( realtimeMediaStream );

    realtime_media_streams_lock.lock();
    realtime_media_streams.remove( realtimeMediaStream );
    if(realtime_media_streams.empty())
    {
        /* End the thread */
        kill = true;
    }
    realtime_media_streams_lock.unlock();
}

void Rtp_Receiver::stop()
{
    kill = true;
}

void Rtp_Receiver::join()
{
    if( !thread )
        return;

    thread->join();
    delete thread;
    thread = NULL;
}

uint16_t Rtp_Receiver::get_port()
{
    return external_port;
}

uint16_t Rtp_Receiver::get_rtcp_port()
{
    return external_rtcp_port;
}

SRef<Rtp_Stream*> Rtp_Receiver::get_rtp_stream()
{
    return rtp_stream;
}

void Rtp_Receiver::run()
{
#ifdef DEBUG_OUTPUT
    set_thread_name("Rtp_Receiver::run callId=" + call_id);
#endif
    SRef<SRtp_Packet *> packet;

    SRef<UDP_Socket*> socket = rtp_stream->get_rtp_socket();
    SRef<UDP_Socket*> rtcpsocket = rtp_stream->get_rtcp_socket();
    SRef<Rtcp_Mgr*> rtcpMgr = rtp_stream->get_rtcp_mgr();
    SRef<IPAddress *> fromStored;

    while( !kill ){
        std::list< SRef<Realtime_Media_Stream_Receiver *> >::iterator i;
        fd_set rfds;
        struct timeval tv;
        int ret = -1;

        FD_ZERO( &rfds );
#ifdef WIN32
        FD_SET( (uint32_t) socket->get_fd(), &rfds );
        FD_SET( (uint32_t) rtcpsocket->get_fd(), &rfds );
#else
        FD_SET( socket->get_fd(), &rfds );
        FD_SET( rtcpsocket->get_fd(), &rfds );
#endif


        while( !kill && ret < 0 )
        {
            int fdmax = std::max(socket->get_fd(), rtcpsocket->get_fd());
            tv.tv_sec = 0;
            tv.tv_usec = 100000;
            ret = select( fdmax + 1, &rfds, NULL, NULL, &tv );
            //			std::cout << "Rtp_Receiver::run() trying to select a socket on port " << socket->get_port() << " at " << mtime() << "ms" << std::endl;
            if( ret < 0 )
            {
#ifdef DEBUG_OUTPUT
                //FIXME: do something better
                perror("Rtp_Receiver::run() - select returned -1");
#endif

#ifndef _WIN32_WCE
                if( errno == EINTR )
#else
                if( errno == WSAEINTR )
#endif
                {
                    FD_ZERO( &rfds );
#ifdef WIN32
                    FD_SET( (uint32_t) socket->get_fd(), &rfds );
                    FD_SET( (uint32_t) rtcpsocket->get_fd(), &rfds );
#else
                    FD_SET( socket->get_fd(), &rfds );
                    FD_SET( rtcpsocket->get_fd(), &rfds );
#endif
                    continue;
                }
                else
                {
                    kill = true;
                    break;
                }
            }
        }

        if( kill )
            break;

        if( ret == 0 /* timeout */ )
        {
            //notify the RealtimeMediaStreams of the timeout
            realtime_media_streams_lock.lock();
            for( i = realtime_media_streams.begin(); i != realtime_media_streams.end(); i++ )
            {
                (*i)->handle_rtp_packet( NULL, call_id, NULL );
                SRef<Rtcp_Packet*> rtcpOut = rtcpMgr->timeout();
                if (rtcpOut && fromStored)
                {
                    //rtcpOut->debug_print();
                    rtcpsocket->sendto( **fromStored, (fromStored->get_port()+1), rtcpOut->get_data(), rtcpOut->get_length() );
                }
            }
            realtime_media_streams_lock.unlock();
            continue;
        }

        SRef<IPAddress *> from = NULL;
        if ( FD_ISSET(socket->get_fd(),&rfds) )
        {
            try{
                packet = SRtp_Packet::read_packet( **socket, from);
                if (!fromStored && from)
                {
                    fromStored = from;
                }
                //we run the "Pre" method early to let RTCP get good timestamps
                if (packet)
                    rtcpMgr->rtp_received_pre(packet);
                else
                    std::cerr << "NULL packet read" << std::endl;
            } catch (Network_Exception &  ){
                std::cerr << "Rtp_Receiver::run() error reading packet, dropping..." << std::endl;
                continue;
            }

            if( !packet )
                continue;

            realtime_media_streams_lock.lock();
            for ( i = realtime_media_streams.begin(); i != realtime_media_streams.end(); i++ )
            {
                std::list<SRef<Codec_Description *> > &codecs = (*i)->get_available_codecs();
                std::list<SRef<Codec_Description *> >::iterator iC;
#ifdef ZRTP_SUPPORT
                int found = 0;
#endif
                for( iC = codecs.begin(); iC != codecs.end(); iC ++ )
                {
                    if ( (*iC)->get_sdp_media_type() == packet->get_header().get_payload_type() || (packet->get_header().get_payload_type() >= 90 && packet->get_header().get_payload_type() <= 110))
                    {
                        (*i)->handle_rtp_packet( packet, call_id, from );
                        SRef<Rtcp_Packet*> rtcpOut = rtcpMgr->rtp_received_post(packet);
                        if (rtcpOut)
                        {
                            //rtcpOut->debug_print();
                            rtcpsocket->sendto( **from, from->get_port()+1, rtcpOut->get_data(), rtcpOut->get_length() );
                        }

#ifdef ZRTP_SUPPORT
                        found = 1;
#endif
                        break;
                    }
                }
#ifdef ZRTP_SUPPORT
                /*
                 * If we come to this point:
                 * no codec was found for this packet.
                 */
                SRef<Zrtp_Host_Bridge_Minisip *> zhb = (*i)->getZrtpHostBridge();

                /*
                 * If the packet was not processed above and it contains an
                 * extension header then check for ZRTP packet.
                 */
                if (!found && zhb && packet->get_header().get_extension()) {
                    (*i)->handle_rtp_packet_ext(packet);
                }
#endif // ZRTP_SUPPORT
            }
            realtime_media_streams_lock.unlock();
        }
        else if(FD_ISSET(rtcpsocket->get_fd(), &rfds))
        {
            SRef<IPAddress *> from = NULL;
            try{
                SRef<Rtcp_Packet*> rtcp;
                rtcp = Rtcp_Packet::read_packet( **rtcpsocket, from);
                if (rtcp)
                {
                    SRef<Rtcp_Packet*> rtcpOut = rtcpMgr->rtcp_received(rtcp);
                    if (rtcpOut)
                    {
                        //rtcpOut->debug_print();
                        rtcpsocket->sendto( **from, from->get_port(), rtcpOut->get_data(), rtcpOut->get_length() );
                    }

                }
            } catch (Network_Exception &  ){
                continue;
            }
        }
    }
    socket = NULL;
    rtcpsocket = NULL;
}
