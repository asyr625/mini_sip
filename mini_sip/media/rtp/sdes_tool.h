#ifndef SDES_TOOL_H
#define SDES_TOOL_H

#include "sdes_item.h"

class Sdes_Tool : public Sdes_Item
{
public:
    Sdes_Tool(void *buildfrom, int max_length);
    virtual ~Sdes_Tool(){}

    int size();

    virtual void debug_print();

    virtual int write(uint8_t*buf);

    virtual std::string get_string() { return tool; }

private:
    std::string tool;
};

#endif // SDES_TOOL_H
