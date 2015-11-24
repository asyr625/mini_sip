#ifndef SDES_EMAIL_H
#define SDES_EMAIL_H
#include <string>
#include "sdes_item.h"

class Sdes_Email : public Sdes_Item
{
public:
    Sdes_Email(void *buildfrom, int max_length);
    virtual ~Sdes_Email(){}

    int size();

    virtual void debug_print();

    virtual int write(uint8_t*buf);

    virtual std::string get_string() { return email; }

private:
    std::string email;
};

#endif // SDES_EMAIL_H
