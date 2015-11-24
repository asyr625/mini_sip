#ifndef RAND_H
#define RAND_H

#include <stddef.h>

#include "sobject.h"
#include "sip_sim.h"

class Rand
{
public:
    Rand();
    static bool randomize(void *buffer, size_t length);
    static bool randomize(void *buffer, size_t length, SRef<Sip_Sim *> sim);
};

#endif // RAND_H
