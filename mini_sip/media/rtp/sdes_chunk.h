#ifndef SDES_CHUNK_H
#define SDES_CHUNK_H
#include <vector>

#include "sdes_item.h"

class Sdes_Chunk
{
public:
    Sdes_Chunk(uint32_t ssrc, Sdes_Item*item);
    Sdes_Chunk(void *buildfrom, int max_length);
    ~Sdes_Chunk();
    void add_item(Sdes_Item * i);
    int size();
    void debug_print();

    int write_data(uint8_t* buf);

    Sdes_Item* get_item(int type);

    uint32_t get_ssrc(){return ssrc_or_csrc;}

private:
    uint32_t ssrc_or_csrc;
    std::vector<Sdes_Item *>sdes_items;
};

#endif // SDES_CHUNK_H
