#ifndef SDES_NOTE_H
#define SDES_NOTE_H

#include "sdes_item.h"

class Sdes_Note : public Sdes_Item
{
public:
    Sdes_Note(void *buildfrom, int max_length);
    virtual ~Sdes_Note(){}

    int size();

    virtual void debug_print();

    virtual int write(uint8_t*buf);

    virtual std::string get_string() { return note; }

private:
    std::string note;
};

#endif // SDES_NOTE_H
