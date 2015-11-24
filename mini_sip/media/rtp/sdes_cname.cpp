#include "sdes_cname.h"

#include <string.h>
Sdes_Cname::Sdes_Cname(std::string name)
    : Sdes_Item(SDES_ITEM_CNAME)
{
    cname = name;
    length = name.length();
}

Sdes_Cname::Sdes_Cname(void *buildfrom, int max_length)
    : Sdes_Item(SDES_ITEM_CNAME)
{
    unsigned char *lengthptr = (unsigned char *)buildfrom;
    lengthptr++;
    length=*lengthptr;

    char *cptr = (char *)buildfrom;
    my_assert(*cptr == SDES_ITEM_CNAME);
    cname = "";

    cptr += 2;
    for (int i = 0; i < *lengthptr; i++)
    {
        cname += *cptr;
        cptr++;
    }
}

int Sdes_Cname::size()
{
    return 2 + (int)cname.length();
}

void Sdes_Cname::debug_print()
{
    std::cerr << "SDES CNAME:"<< std::endl;
    std::cerr << "\tlength: "<< length << std::endl;
    std::cerr << "\tcname: "<< cname << std::endl;
}

int Sdes_Cname::write(uint8_t* buf)
{
    buf[0] = 1;
    buf[1] = cname.length();
    memcpy(buf+2, cname.c_str(), cname.length() );
    my_assert(buf[0] == 1);
    return cname.length() + 2;
}
