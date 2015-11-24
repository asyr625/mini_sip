#include "sdes_unknow.h"

Sdes_Unknow::Sdes_Unknow(void *buildfrom, int max_length)
    : Sdes_Item(SDES_ITEM_UNKNOWN)
{
    unsigned char *lengthptr = (unsigned char *)buildfrom;
    lengthptr++;
    length = *lengthptr;
}


int Sdes_Unknow::size()
{
    return length + 2;
}

void Sdes_Unknow::debug_print()
{
    std::cerr << "\tunknown: ?"<< std::endl;
}

int Sdes_Unknow::write(uint8_t*)
{
    my_assert(false);//Is is a bug to arrive at this point
    return -1;
}
