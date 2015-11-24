#include "sdes_phone.h"

Sdes_Phone::Sdes_Phone(void *buildfrom, int max_length)
    : Sdes_Item(SDES_ITEM_PHONE)
{
    unsigned char *lengthptr = (unsigned char *)buildfrom;
    lengthptr++;
    length=*lengthptr;

    char *cptr = (char *)buildfrom;
    my_assert(*cptr == SDES_ITEM_PHONE);
    phone="";

    cptr += 2;
    for (int i = 0; i < *lengthptr; i++)
    {
        phone += *cptr;
        cptr ++;
    }
#ifdef DEBUG_OUTPUT
    std::cerr << "In SDES_PHONE: Parsed string name to <"<< phone << ">"<< std::endl;
#endif
}


int Sdes_Phone::size()
{
    return 2 + (int)phone.length();
}

void Sdes_Phone::debug_print()
{
    std::cerr << "SDES PHONE:"<< std::endl;
    std::cerr << "\tlength: "<< length << std::endl;
    std::cerr << "\tname: "<< phone << std::endl;
}

int Sdes_Phone::write(uint8_t*buf)
{
    buf[0] = 4;
    buf[1] = phone.length();
    memcpy(buf+2, phone.c_str(), phone.length() );
    return phone.length() + 2;
}
