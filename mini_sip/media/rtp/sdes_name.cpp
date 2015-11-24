#include "sdes_name.h"

Sdes_Name::Sdes_Name(void *buildfrom, int max_length)
    : Sdes_Item(SDES_ITEM_NAME)
{
    unsigned char *lengthptr = (unsigned char *)buildfrom;
    lengthptr ++;
    length = *lengthptr;

    char *cptr = (char *)buildfrom;
    my_assert(*cptr == SDES_ITEM_NAME);

    name="";

    cptr += 2;
    for (int i = 0; i < *lengthptr; i++)
    {
        name += *cptr;
        cptr ++;
    }
#ifdef DEBUG_OUTPUT
    std::cerr << "In SDES_NAME: Parsed string name to <"<< name << ">"<< std::endl;
#endif
}


int Sdes_Name::size()
{
    return 2 + (int)name.length();
}

void Sdes_Name::debug_print()
{
    std::cerr << "SDES NAME:"<< std::endl;
    std::cerr << "\tlength: "<< length << std::endl;
    std::cerr << "\tname: "<< name << std::endl;
}

int Sdes_Name::write(uint8_t* buf)
{
    buf[0] = 2;
    buf[1] = name.length();
    memcpy( buf+2, name.c_str(), name.length() );
    return name.length() + 2;
}
