#include "sdes_note.h"


Sdes_Note::Sdes_Note(void *buildfrom, int max_length)
    : Sdes_Item(SDES_ITEM_NOTE)
{
    unsigned char *lengthptr = (unsigned char *)buildfrom;
    lengthptr++;
    length=*lengthptr;

    char *cptr = (char *)buildfrom;
    my_assert(*cptr == SDES_ITEM_NOTE);

    note="";

    cptr += 2;
    for (int i = 0; i < *lengthptr; i++)
    {
        note += *cptr;
        cptr ++;
    }
#ifdef DEBUG_OUTPUT
    std::cerr << "In SDES_NOTE: Parsed string name to <"<< note << ">"<< std::endl;
#endif
}

int Sdes_Note::size()
{
    return 2 + (int)note.length();
}

void Sdes_Note::debug_print()
{
    std::cerr << "SDES NOTE:"<< std::endl;
    std::cerr << "\tlength: "<< length << std::endl;
    std::cerr << "\tname: "<< note << std::endl;
}

int Sdes_Note::write(uint8_t*buf)
{
    buf[0] = 7;
    buf[1] = note.length();
    memcpy(buf+2, note.c_str(), note.length() );
    return note.length() + 2;
}
