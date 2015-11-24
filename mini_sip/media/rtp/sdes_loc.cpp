#include "sdes_loc.h"
#include <string.h>
Sdes_Loc::Sdes_Loc(std::string location)
    : Sdes_Item(SDES_ITEM_LOC)
{
    loc = location;
    length = location.length();
}

Sdes_Loc::Sdes_Loc(void *buildfrom, int max_length)
    : Sdes_Item(SDES_ITEM_LOC)
{
    unsigned char *lengthptr = (unsigned char *)buildfrom;
    lengthptr++;
    length=*lengthptr;

    char *cptr = (char *)buildfrom;
    my_assert(*cptr == SDES_ITEM_LOC);
    loc="";

    cptr += 2;
    for (int i = 0 ; i < *lengthptr; i++)
    {
        loc += *cptr;
        cptr ++;
    }
}


int Sdes_Loc::size()
{
    return 2 + (int)loc.length();
}

void Sdes_Loc::debug_print()
{
    std::cerr << "SDES LOC:"<< std::endl;
    std::cerr << "\tlength: "<< length << std::endl;
    std::cerr << "\tname: "<< loc << std::endl;
}

int Sdes_Loc::write(uint8_t*buf)
{
    buf[0] = 5;
    buf[1] = loc.length();
    memcpy(buf+2, loc.c_str(), loc.length() );
    return loc.length() + 2;
}
