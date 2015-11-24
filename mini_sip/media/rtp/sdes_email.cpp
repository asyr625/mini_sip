#include "sdes_email.h"

#include <string.h>
Sdes_Email::Sdes_Email(void *buildfrom, int max_length)
    : Sdes_Item(SDES_ITEM_EMAIL)
{
    unsigned char *lengthptr = (unsigned char *)buildfrom;
    lengthptr++;
    length=*lengthptr;

    char *cptr = (char *)buildfrom;
    my_assert(*cptr == SDES_ITEM_EMAIL);
    email="";

    cptr += 2;
    for (int i=0 ; i < *lengthptr; i++)
    {
        email += *cptr;
        cptr ++;
    }
#ifdef DEBUG_OUTPUT
    std::cerr << "In SDES_EMAIL: Parsed string name to <"<< email << ">"<< std::endl;
#endif
}

int Sdes_Email::size()
{
    return 2 + (int)email.length();
}

void Sdes_Email::debug_print()
{
    std::cerr << "SDES EMAIL:"<< std::endl;
    std::cerr << "\tlength: "<< length << std::endl;
    std::cerr << "\tname: "<< email << std::endl;
}

int Sdes_Email::write(uint8_t* buf)
{
    buf[0] = 3;
    buf[1] = email.length();
    memcpy(buf+2, email.c_str(), email.length() );
    return email.length()+2;
}
