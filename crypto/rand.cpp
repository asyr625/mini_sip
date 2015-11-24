#include "rand.h"

#include <openssl/rand.h>

#include <string.h>

Rand::Rand()
{
}

bool Rand::randomize(void *buffer, size_t length)
{
    return RAND_bytes((unsigned char*)buffer, (int)length) != 0;
}

bool Rand::randomize(void * buffer, size_t length, SRef<Sip_Sim *> sim)
{
    unsigned char * tempBufferPtr = new unsigned char[16];
    size_t index = 0;
    size_t left = length;
    while(left > 16)
    {
        if(sim->get_random_value(tempBufferPtr, 16))
        {
            memcpy(&((unsigned char *)buffer)[index], tempBufferPtr, 16);
            index += 16;
            left -= 16;
        }
        else
        {
            delete [] tempBufferPtr;
            return false;
        }
    }
    if(sim->get_random_value(tempBufferPtr, 16))
    {
        memcpy(&((unsigned char *)buffer)[index], tempBufferPtr, left);
        delete [] tempBufferPtr;
        return true;
    }
    else
    {
        delete [] tempBufferPtr;
        return false;
    }
}
