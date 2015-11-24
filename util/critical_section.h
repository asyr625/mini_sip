#ifndef CRITICAL_SECTION_H
#define CRITICAL_SECTION_H

#include "mutex.h"

class Critical_Section
{
public:
    Critical_Section(Mutex &mutex);
    ~Critical_Section();

private:
    Mutex &_mutex;
};

#endif // CRITICAL_SECTION_H
