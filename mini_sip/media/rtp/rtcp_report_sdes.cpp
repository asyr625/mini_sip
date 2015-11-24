#include "rtcp_report_sdes.h"

Rtcp_Report_Sdes::Rtcp_Report_Sdes(Sdes_Chunk*chunk)
    : Rtcp_Report(202)
{
    chunks.push_back(chunk);
}

Rtcp_Report_Sdes::Rtcp_Report_Sdes(void * build_from, int max_length)
    : Rtcp_Report(202)
{
    if (max_length<4)
    {
        cerr <<"Too short RTCP SDES report (in RtcpReportSDES constructor) (size="<<max_length<<")"<< endl;
        exit(1);
    }
    parse_header(build_from,max_length);

    max_length = length * 4;
    int i = 4;
    for (unsigned j = 0; j < rc_sc; j++)
    {
        Sdes_Chunk *chunk = new Sdes_Chunk(& (((char*)build_from)[i]), max_length-i);
        chunks.push_back(chunk);
        i += chunk->size();
    }
}

Rtcp_Report_Sdes::~Rtcp_Report_Sdes()
{
    for (unsigned i=0; i<chunks.size(); i++)
        delete chunks[i];
}

int Rtcp_Report_Sdes::get_length()
{
    int len = 0;
    for (unsigned j = 0; j < chunks.size(); j++)
        len += chunks[j]->size();
    return len + 4;
}

void Rtcp_Report_Sdes::write_data(char* buf)
{
    int i = 0;
    rc_sc = chunks.size();
    i += write_header(buf);
    for (unsigned j=0; j<chunks.size(); j++)
    {
        i+=chunks[j]->write_data((uint8_t*)&buf[i]);
    }
}

void Rtcp_Report_Sdes::debug_print()
{
    std::cerr << "RTCP SDES report:"<< std::endl;
    for (unsigned i=0; i<chunks.size(); i++)
        chunks[i]->debug_print();
}

Sdes_Item* Rtcp_Report_Sdes::get_item(int type, uint32_t* ssrc_out)
{
    for (unsigned j = 0; j < chunks.size(); j++)
    {
        Sdes_Item *item = chunks[j]->get_item(type);
        if (item)
        {
            if (ssrc_out)
                *ssrc_out = chunks[j]->get_ssrc();
            return item;
        }
    }
    return NULL;
}
