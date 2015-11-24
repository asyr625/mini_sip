#include<unistd.h>

#include "rtp_stream.h"
#include "network_exception.h"
#include "dbg.h"

#define RTP_RECEIVER_MAX_RETRIES 15

int Rtp_Stream::rtp_port_range_start = 30000;
int Rtp_Stream::rtp_port_range_end = 35000;

Rtp_Stream::Rtp_Stream(bool useIpv6, const std::string &_callID,
                       IStreams_Player_Report_Timestamps *_streamsPlayer, Session* session)
{
    int portretry = 0;
    if (rtp_port_range_start < 0 || rtp_port_range_start > 65535)
        rtp_port_range_start = 30000;
    //TODO: more range sanity checks
    int rtpRange = rtp_port_range_end - rtp_port_range_start;
    if (rtpRange <= 0) //sanity check since configuration values are not checked elsewhere
        rtpRange = 1024;
    for (; portretry < RTP_RECEIVER_MAX_RETRIES; portretry++ )
    {
        //generate a random port, even number, in the given range
        float randPartial =  (float)rand()  /  RAND_MAX;
        int port = (int) (rtpRange * randPartial );
        port = (int) ( 2 * (int)(port/2 ) ); //turn this into an even number
        port += rtp_port_range_start; //add the min port to set it within the range
        try{
            rtp_socket = new UDP_Socket( port, useIpv6 );
            if (rtp_socket)
            {
                rtcp_socket = new UDP_Socket(port+1, useIpv6 );
            }
            if( rtp_socket && rtcp_socket )
            {
                break;
            }
            rtp_socket = NULL;
            rtcp_socket = NULL;
        }
        catch( Network_Exception &  )
        {
            // FIXME: do something nice
            //                      merr << "Minisip could not create a UDP socket!" << end;
            //                      merr << "Check your network settings." << end;
            //                      exit( 1 );
#ifdef DEBUG_OUTPUT
            my_err << "RtpReceiver: Could not create UDP socket on port "<<port << std::endl;
#endif
        }
    }
    if( portretry == RTP_RECEIVER_MAX_RETRIES && !rtp_socket )
    {
        my_err << "Minisip could not create a UDP socket!" << std::endl;
        my_err << "Check your network settings." << std::endl << "Quitting badly" << std::endl;
        exit( 1 );
    }
    rtcp_mgr = new Rtcp_Mgr(_callID, _streamsPlayer, session);
}

Rtp_Stream::~Rtp_Stream()
{
}

void Rtp_Stream::send(const SRef<Rtp_Packet *> &packet, const IPAddress &to_addr, const uint16_t &port)
{
    if(rtp_socket)
    {
        packet->sendto(**rtp_socket, to_addr, port);
        if(rtcp_mgr)
        {
            SRef<Rtcp_Packet*> rtcpPacket = rtcp_mgr->rtp_sent(packet);
            if(rtcpPacket)
            {
                //cerr <<"OUT RTCP (timeout): "<<endl;
                //rtcpPacket->debug_print();
                rtcp_socket->sendto(to_addr, port+1, rtcpPacket->get_data(), rtcpPacket->get_length());
            }
        }
    }
    usleep(100);
}

SRef<UDP_Socket*> Rtp_Stream::get_rtp_socket()
{
    return rtp_socket;
}

SRef<UDP_Socket*> Rtp_Stream::get_rtcp_socket()
{
    return rtcp_socket;
}

SRef<Rtcp_Mgr*> Rtp_Stream::get_rtcp_mgr()
{
    return rtcp_mgr;
}
