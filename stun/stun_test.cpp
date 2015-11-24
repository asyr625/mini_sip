#ifdef LINUX
#	include<sys/select.h>
#endif

#ifdef WIN32
#	include<winsock2.h>
#endif

#include<errno.h>
#include<stdio.h>
#include<stdlib.h>

#include "stun_test.h"
#include "my_error.h"

Stun_Message *Stun_Test::test(IPAddress *addr, uint16_t port, UDP_Socket &sock, bool changeIP, bool changePort)
{
    int timeoutIndex = 0;
    short timeouts[9] = {100, 200, 400, 800, 1600,1600,1600,1600,1600};

    int length;

    Stun_Message br(Stun_Message::BINDING_REQUEST);
    //	Stun_MessageBindingRequest br;
    br.add_attribute(new Stun_Attribute_Change_Request(changeIP,changePort));
    unsigned char *data = br.get_message_data( length );

    bool done = false;
    Stun_Message *msg = NULL;

    do{
        /*int slen = */
        sock.sendto( *addr, port, data, length );
        //cerr << "Sent "<< slen<< " bytes" << endl;

        //struct pollfd p;
        //p.fd = sock.getFd();
        //p.events = POLLIN;

        fd_set set;
        FD_ZERO(&set);

#ifdef WIN32
        FD_SET( (uint32_t) sock.get_fd(),&set );
#else
        FD_SET( sock.get_fd(), 		&set );
#endif

        struct timeval tv;
        tv.tv_sec = timeouts[timeoutIndex] / 1000;
        tv.tv_usec = ( timeouts[timeoutIndex] % 1000 ) * 1000;

        //cerr << "Waiting for max "<<timeouts[timeoutIndex]<<" ms"<<endl;
        //int avail = poll(&p,1,timeouts[timeoutIndex]);
        int avail = select( sock.get_fd() + 1, &set, NULL, NULL, &tv );
        //cerr <<"After poll, return value is "<<avail<<endl;
        if (avail < 0)
        {
#ifndef _WIN32_WCE
            if (errno!=EINTR){
#else
            if (errno!=WSAEINTR){
#endif
                my_error("Error when using poll:");
                exit(1);
            }
            else
            {
                //cerr << "Signal occured in wait_packet"<<endl;
            }
        }

        if( avail>0 )
        {
            unsigned char resp[2048];

            int rlen = sock.recv(resp, 2048);

            //msg=Stun_Message::parse_message(resp,rlen);
            msg = new Stun_Message(resp,rlen);

            if( msg->same_transaction_id(br) )
            {
                //cerr <<"Accepted: "<<msg->getDesc()<<endl;
                done=true;
            }else{
                //cerr <<"Discarded: "<<msg->getDesc()<<endl;
            }
        }
        if( timeoutIndex >= 8 )
            done = true;
        timeoutIndex++;
    }while( !done );

    return msg;
}
