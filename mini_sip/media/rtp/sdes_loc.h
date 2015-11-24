#ifndef SDES_LOC_H
#define SDES_LOC_H

#include "sdes_item.h"

class Sdes_Loc : public Sdes_Item
{
public:
    Sdes_Loc(std::string location);
    Sdes_Loc(void *buildfrom, int max_length);
    virtual ~Sdes_Loc(){}

    int size();

    virtual void debug_print();

    virtual int write(uint8_t*);

    virtual std::string get_string() { return loc; }
private:
    std::string loc;
};

#endif // SDES_LOC_H
