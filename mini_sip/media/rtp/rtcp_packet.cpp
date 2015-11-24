#include "rtcp_packet.h"

#include "rtcp_packet.h"
#include "rtcp_report.h"
#include "rtcp_report_fir.h"
#include "rtcp_report_sr.h"
#include "rtcp_report_rr.h"
#include "rtcp_report_sdes.h"
#include "rtcp_report_app_view.h"
#include "rtcp_report_app_camctrl.h"
#include "rtcp_report_app_face.h"

#include "my_error.h"
#include "string_utils.h"
#include "dbg.h"

#ifdef DEBUG_OUTPUT
#include<iostream>
#endif

using namespace std;

vector<Rtcp_Report_Factory_Func_Ptr> Rtcp_Packet::report_factories = vector<Rtcp_Report_Factory_Func_Ptr>();

Rtcp_Packet::Rtcp_Packet()
    : binary_dirty(true), binary_data(NULL)
{
}

Rtcp_Packet::~Rtcp_Packet()
{
    for (unsigned i=0; i<reports.size(); i++)
        delete reports[i];

    if (binary_data)
    {
        delete [](byte_t*)binary_data;
        binary_data = NULL;
    }
}

SRef<Rtcp_Packet*> Rtcp_Packet::read_packet(UDP_Socket &sock, SRef<IPAddress *>&from, int timeout )
{
    int fromport;
    int n = sock.recvfrom(sock.receive_buffer, UDP_SIZE, from, fromport);
    if (n < 0)
    {
        my_error("recvfrom:");
        return NULL;
    }
    return read_packet(sock.receive_buffer, n);
}

SRef<Rtcp_Packet*> Rtcp_Packet::read_packet(unsigned char *buf, int len)
{
    if (len <= 0 || buf == NULL)
        return NULL;

    SRef<Rtcp_Packet*> rtcp = new Rtcp_Packet;
    rtcp->binary_dirty = true;

    int i = 0;
    //for all RTCP packets in the UDP packet
    while (i < len - 1 )
    {
        //all RTCP packets have the same header
        int type = buf[i+1];
        int length_bytes = (U16_AT(&buf[i+2])+1)*4;

        Rtcp_Report* report=NULL;
        switch(type)
        {
        case PACKET_TYPE_FIR:
            report = new Rtcp_Report_Fir(&buf[i], length_bytes);
            break;
        case PACKET_TYPE_SR:
            report = new Rtcp_Report_SR(&buf[i], length_bytes);
            break;
        case PACKET_TYPE_RR:
            report = new Rtcp_Report_RR(&buf[i], length_bytes);
            break;
        case PACKET_TYPE_SDES:
            report = new Rtcp_Report_Sdes(&buf[i], length_bytes);
            break;
            //			case PACKET_TYPE_BYE:
            //				report = new RtcpReportBYE(buf, length_bytes);
            //				break;
        case PACKET_TYPE_APP:
        {
            if (buf[i+8]=='V' && buf[i+9]=='I' && buf[i+10]=='E' && buf[i+11]=='W')
                report = new Rtcp_Report_App_View(buf, length_bytes);
            else if (buf[i+8]=='C' && buf[i+9]=='C' && buf[i+10]=='T' && buf[i+11]=='L'){
                report = new Rtcp_Report_App_Camctrl(buf, length_bytes);
            }else if (buf[i+8]=='F' && buf[i+9]=='A' && buf[i+10]=='C' && buf[i+11]=='E'){
                report = new Rtcp_Report_App_Face(buf, length_bytes);
            }else{
                my_dbg<<"WARNING: uknown RTCP APP packet ignored"<<endl;
            }

            break;
        }

            //TODO: PACKET_TYPE_XR

        default:
            break;
        };
        vector<Rtcp_Report_Factory_Func_Ptr>::iterator fi;
        if (!report)
        {
            for (fi=report_factories.begin(); fi!=report_factories.end() && !report ; fi++)
            {
                report = (*fi)(buf,length_bytes);
            }
        }

        if (report)
            rtcp->add_report(report);
        i += length_bytes;
    }
    return rtcp;
}

std::vector<Rtcp_Report *> &Rtcp_Packet::get_reports()
{
    return reports;
}

void Rtcp_Packet::add_report(Rtcp_Report *report)
{
    binary_dirty = true;
    reports.push_back(report);
}

void Rtcp_Packet::debug_print()
{
    cerr << "__RTCP_packet__";
    for (unsigned i=0; i<reports.size(); i++)
    {
        cerr <<"_report "<< i+1<< "_"<< endl;
        reports[i]->debug_print();
    }
}

const void *Rtcp_Packet::get_data()
{
    update_binary();
    return binary_data;
}

int Rtcp_Packet::get_length()
{
    update_binary();
    return binary_length;
}

void Rtcp_Packet::update_binary()
{
    if (binary_dirty)
    {
        binary_length = 0;
        for (unsigned i=0; i<reports.size(); i++)
        {
            binary_length += reports[i]->get_length();
        }
        if (binary_data)
            delete [](byte_t*)binary_data;
        binary_data = new byte_t[binary_length];
        char *data = (char*)binary_data;

        data[binary_length-1] = 0; //if padding, make sure it is zero
        data[binary_length-2] = 0;
        data[binary_length-3] = 0;

        int j = 0;
        for (unsigned i=0; i<reports.size(); i++)
        {
            reports[i]->write_data(&data[j]);
            j+=reports[i]->get_length();
        }
        binary_dirty = false;
    }
}
