#ifndef SDES_UNKNOW_H
#define SDES_UNKNOW_H

#include "sdes_item.h"

class Sdes_Unknow : public Sdes_Item
{
public:
    Sdes_Unknow(void *buildfrom, int max_length);
    virtual ~Sdes_Unknow(){}

    int size();

    virtual void debug_print();

    virtual int write(uint8_t*);

    virtual std::string get_string() { return "UNKOWN"; }
};

#endif // SDES_UNKNOW_H
