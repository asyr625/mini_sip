#ifndef SDES_ITEM_H
#define SDES_ITEM_H

#include <iostream>
#include<string.h>

#include "my_types.h"
#include "my_assert.h"

#define SDES_ITEM_UNKNOWN 0
#define SDES_ITEM_CNAME 1
#define SDES_ITEM_NAME 2
#define SDES_ITEM_EMAIL 3
#define SDES_ITEM_PHONE 4
#define SDES_ITEM_LOC 5
#define SDES_ITEM_TOOL 6
#define SDES_ITEM_NOTE 7

class Sdes_Item
{
public:
    Sdes_Item(int t);
    virtual ~Sdes_Item() {}

    virtual int size() = 0;
    static Sdes_Item *build_from(void *from,int max_length);

    virtual void debug_print() = 0;

    virtual int write(uint8_t* buf) = 0;

    virtual std::string get_string() = 0;

    int get_type() { return type; }

protected:
    int type;
    unsigned length;
};

#endif // SDES_ITEM_H
