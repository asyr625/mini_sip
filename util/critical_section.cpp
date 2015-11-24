#include "critical_section.h"

Critical_Section::Critical_Section(Mutex &mutex): _mutex(mutex)
{
    _mutex.lock();
}

Critical_Section::~Critical_Section()
{
    _mutex.unlock();
}
