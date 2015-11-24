#include <iostream>

#include "sdes_item.h"
#include "sdes_cname.h"
#include "sdes_email.h"
#include "sdes_loc.h"
#include "sdes_name.h"
#include "sdes_note.h"
#include "sdes_phone.h"
#include "sdes_tool.h"
#include "sdes_unknow.h"


Sdes_Item::Sdes_Item(int t)
    : type(t)
{

}


Sdes_Item *Sdes_Item::build_from(void *from,int max_length)
{
    unsigned char* cptr = (unsigned char *)from;

    switch (*cptr)
    {
        case SDES_ITEM_CNAME:
            return new Sdes_Cname(from, max_length);
            break;
        case SDES_ITEM_NAME:
            return new Sdes_Name(from, max_length);
            break;
        case SDES_ITEM_EMAIL:
            return new Sdes_Email(from, max_length);
            break;
        case SDES_ITEM_PHONE:
            return new Sdes_Phone(from, max_length);
            break;
        case SDES_ITEM_LOC:
            return new Sdes_Loc(from, max_length);
            break;
        case SDES_ITEM_TOOL:
            return new Sdes_Tool(from, max_length);
            break;
        case SDES_ITEM_NOTE:
            return new Sdes_Note(from, max_length);
            break;
        default:
            return new Sdes_Unknow(from, max_length);
            ;
#ifdef DEBUG_OUTPUT
            std::cerr <<"ERROR: Parsed SDES item of unknown type ("<<(int)*cptr<<")"<< std::endl;
#endif
    }
    return NULL;
}
