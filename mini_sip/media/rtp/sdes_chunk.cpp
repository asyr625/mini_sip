#include "sdes_chunk.h"

#include "mini_defines.h"

#include <arpa/inet.h>

Sdes_Chunk::Sdes_Chunk(uint32_t ssrc, Sdes_Item* item)
{
    sdes_items.push_back(item);
    ssrc_or_csrc = ssrc;
}

Sdes_Chunk::Sdes_Chunk(void *buildfrom, int max_length)
{
    unsigned int *iptr=(unsigned int *)buildfrom;
    unsigned char *cptr=(unsigned char *)buildfrom;

    ssrc_or_csrc = U32_AT(iptr);

    int start = 4;
    Sdes_Item *sdes_item;
    while (start < max_length-4-3) //FIX: check if correct
    {
        sdes_item = Sdes_Item::build_from(&cptr[start], max_length-start);
        sdes_items.push_back(sdes_item);
        start+=sdes_item->size();
    }
}

Sdes_Chunk::~Sdes_Chunk()
{
    int n=(int)sdes_items.size();
    for (int i=0; i<n;i++)
        delete sdes_items[i];
}

void Sdes_Chunk::add_item(Sdes_Item * i)
{
    sdes_items.push_back(i);
}

int Sdes_Chunk::size()
{
    int ret = 5; //ssrc + null terminator
    for (unsigned i=0; i<sdes_items.size(); i++)
        ret += sdes_items[i]->size();
    int pad = 0;
    if (ret%4 != 0)
        pad = 4-ret%4;
    return ret + pad;
}

void Sdes_Chunk::debug_print()
{
    std::cerr << "SDES chunk: "<< std::endl;
    std::cerr << "\tssrc/csrc: "<< ssrc_or_csrc<< std::endl;
    for (unsigned i=0; i< sdes_items.size(); i++)
        sdes_items[i]->debug_print();
}

int Sdes_Chunk::write_data(uint8_t* buf)
{
    uint8_t*tmp = buf;
    uint32_t* u32 = (uint32_t*)buf;
    u32[0] = htonl(ssrc_or_csrc);
    buf += 4;
    for (unsigned i=0; i< sdes_items.size(); i++)
    {
        buf += sdes_items[i]->write(buf);
    }
    *buf = 0;//marks end of items in chunk

    return size();
}

Sdes_Item* Sdes_Chunk::get_item(int type)
{
    for (unsigned i=0; i< sdes_items.size(); i++)
    {
        if (sdes_items[i]->get_type() ==type)
            return sdes_items[i];
    }
    return NULL;
}
