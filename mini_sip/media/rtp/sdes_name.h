#ifndef SDES_NAME_H
#define SDES_NAME_H

#include "sdes_item.h"

class Sdes_Name : public Sdes_Item
{
public:
    Sdes_Name(void *buildfrom, int max_length);
    virtual ~Sdes_Name(){}

    int size();

    virtual void debug_print();

    virtual int write(uint8_t* buf);

    virtual std::string get_string() { return name; }
private:
    std::string name;
};

#endif // SDES_NAME_H
