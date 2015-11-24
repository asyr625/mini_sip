#ifndef SMART_CARD_EXCEPTION_H
#define SMART_CARD_EXCEPTION_H

#include "exception.h"

class Smart_Card_Exception : public Exception
{
public:
    Smart_Card_Exception(const char * message) : Exception(message) {}
    virtual ~Smart_Card_Exception() throw(){}
};

#endif // SMART_CARD_EXCEPTION_H
