#ifndef RTCP_PACKET_H
#define RTCP_PACKET_H

#include <vector>
#include <iostream>
using namespace std;

#include "sobject.h"
#include "rtcp_report.h"
#include "udp_socket.h"

typedef Rtcp_Report* (*Rtcp_Report_Factory_Func_Ptr)(unsigned char* buf, int buflen);

class Rtcp_Packet : public SObject
{
public:
    Rtcp_Packet();
    ~Rtcp_Packet();

    static SRef<Rtcp_Packet*> read_packet(UDP_Socket &sock, SRef<IPAddress *>&from, int timeout = -1);
    static SRef<Rtcp_Packet*> read_packet(unsigned char *buf, int len);

    std::vector<Rtcp_Report *> &get_reports();
    void add_report(Rtcp_Report *report);

    void debug_print();

    const void *get_data();
    int get_length();

    static void add_rtcp_factory(Rtcp_Report_Factory_Func_Ptr f)
    {
        report_factories.push_back(f);
    }

private:
    static std::vector<Rtcp_Report_Factory_Func_Ptr> report_factories;

    void update_binary();
    std::vector<Rtcp_Report *> reports;
    int type;
    bool binary_dirty;
    void* binary_data;
    int binary_length;
};

#endif // RTCP_PACKET_H
