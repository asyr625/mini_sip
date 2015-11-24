#include "rtcp_report_sr.h"

Rtcp_Report_SR::Rtcp_Report_SR(unsigned ssrc)
    : Rtcp_Report(0), sender_ssrc(ssrc)
{

}

Rtcp_Report_SR::Rtcp_Report_SR(void * buildfrom, int max_length)
    : Rtcp_Report(0)
{
    if (max_length < 4)
    {
        std::cerr <<"Too short RTCP SR report (in RtcpReportSR constructor) (size="<<max_length<<")"<< std::endl;
        exit(1);
    }
    parse_header(buildfrom, max_length);

    int *iptr = &((int *)buildfrom)[1];
    sender_ssrc = U32_AT( iptr );

//	cerr << "Found SR report with content length of "<< length;
    sender_info = new Rtcp_Report_Sender_Info(& ((char*)buildfrom)[8], max_length-8);

    int i = 8;
    if(sender_info)
        i += sender_info->get_length();
    for (unsigned j=0; j<rc_sc; j++)
    {
        Rtcp_Report_Reception_Block block(& ((char*)buildfrom)[i], max_length-i);
        reception_blocks.push_back(block);
        i+=block.get_length();
    }
}

Rtcp_Report_SR::~Rtcp_Report_SR()
{
}

int Rtcp_Report_SR::get_length()
{
    int totsize = 8;
    if(sender_info)
        totsize += sender_info->get_length();
    for (unsigned i = 0; i< reception_blocks.size(); i++)
        totsize += reception_blocks[i].get_length();
    return totsize;
}

void Rtcp_Report_SR::write_data(char* buf)
{
    int i=0;
    uint32_t* ub = (uint32_t*)buf;
    my_assert(get_length()%4==0);
    length = get_length()/4;
    i+=write_header(buf);
    ub[1]=hton32(sender_ssrc);
    i+=4;

    if(sender_info) {
        sender_info->write_data(&buf[i]);
        i += sender_info->get_length();
    }

    for(unsigned j=0; j< reception_blocks.size(); j++)
    {
        reception_blocks[j].write_data(&buf[i]);
        i += reception_blocks[i].get_length();
    }
}

void Rtcp_Report_SR::debug_print()
{
    std::cerr<<"RTCP SR report:"<< std::endl;
    std::cerr.setf( std::ios::hex, std::ios::basefield );
    std::cerr<< "\tsender_ssrc: "<< sender_ssrc<< std::endl;
    std::cerr.setf(std::ios::dec, std::ios::basefield );
    if(sender_info)
        sender_info->debug_print();
    for (unsigned i=0; i<reception_blocks.size(); i++)
        reception_blocks[i].debug_print();
}

SRef<Rtcp_Report_Sender_Info *> Rtcp_Report_SR::get_sender_info()
{
    return sender_info;
}

void Rtcp_Report_SR::set_sender_info(const Rtcp_Report_Sender_Info &info)
{
    sender_info = new Rtcp_Report_Sender_Info(info);
}

int Rtcp_Report_SR::get_nreport_blocks()
{
    return (int)reception_blocks.size();
}

Rtcp_Report_Reception_Block &Rtcp_Report_SR::get_reception_block(int i)
{
    return reception_blocks[i];
}

void Rtcp_Report_SR::add_reception_block(Rtcp_Report_Reception_Block &b)
{
    reception_blocks.push_back(b);
    rc_sc = reception_blocks.size();
}

unsigned int Rtcp_Report_SR::get_sender_ssrc()
{
    return sender_ssrc;
}
