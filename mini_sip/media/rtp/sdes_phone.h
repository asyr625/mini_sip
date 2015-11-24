#ifndef SDES_PHONE_H
#define SDES_PHONE_H

#include "sdes_item.h"

class Sdes_Phone : public Sdes_Item
{
public:
    Sdes_Phone(void *buildfrom, int max_length);
    virtual ~Sdes_Phone() {}

    int size();

    virtual void debug_print();

    virtual int write(uint8_t*buf);

    virtual std::string get_string() { return phone; }

private:
    std::string phone;
};

#endif // SDES_PHONE_H
