#ifndef SDES_CNAME_H
#define SDES_CNAME_H

#include "sdes_item.h"

class Sdes_Cname : public Sdes_Item
{
public:
    Sdes_Cname(std::string name);
    Sdes_Cname(void *buildfrom, int max_length);
    virtual ~Sdes_Cname(){}

    int size();

    virtual void debug_print();

    virtual int write(uint8_t* buf);

    virtual std::string get_string() { return cname; }

private:
    std::string cname;
};

#endif // SDES_CNAME_H
