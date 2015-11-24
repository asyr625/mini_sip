#include "sdes_tool.h"

Sdes_Tool::Sdes_Tool(void *buildfrom, int max_length)
    : Sdes_Item(SDES_ITEM_TOOL)
{
    unsigned char *lengthptr = (unsigned char *)buildfrom;
    lengthptr++;
    length=*lengthptr;

    char *cptr = (char *)buildfrom;
    my_assert(*cptr == SDES_ITEM_TOOL);
    tool="";

    cptr += 2;
    for (int i = 0; i < *lengthptr; i++)
    {
        tool += *cptr;
        cptr ++;
    }
#ifdef DEBUG_OUTPUT
    std::cerr << "In SDES_TOOL: Parsed string name to <"<< tool << ">"<< std::endl;
#endif
}

int Sdes_Tool::size()
{
    return 2+(int)tool.length();
}

void Sdes_Tool::debug_print()
{
    std::cerr << "SDES TOOL:"<< std::endl;
    std::cerr << "\tlength: "<< length << std::endl;
    std::cerr << "\tname: "<< tool << std::endl;
}

int Sdes_Tool::write(uint8_t*buf)
{
    buf[0] = 6;
    buf[1] = tool.length();
    memcpy(buf+2, tool.c_str(), tool.length() );
    return tool.length() + 2;
}
