#ifndef RTCP_REPORT_H
#define RTCP_REPORT_H

#include <my_assert.h>
#include <iostream>
#include <stdlib.h>
using namespace std;

#include "mini_defines.h"

#define PACKET_TYPE_FIR 192
#define PACKET_TYPE_SR 200
#define PACKET_TYPE_RR 201
#define PACKET_TYPE_SDES 202
#define PACKET_TYPE_BYE 203
#define PACKET_TYPE_APP 204

#define PACKET_TYPE_XR 207

class Rtcp_Report
{
public:
    Rtcp_Report(unsigned packet_type);
    virtual ~Rtcp_Report(){}

    virtual int get_length() = 0;
    virtual void write_data(char* buf) = 0;

    virtual void debug_print() = 0;
protected:
    void parse_header(void *build_from, int max_length);
    int write_header(char* buf);

    unsigned version;
    unsigned padding;
    unsigned rc_sc;
    unsigned packet_type;
    unsigned length;
};

#endif // RTCP_REPORT_H
