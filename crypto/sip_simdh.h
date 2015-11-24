#ifndef SIP_SIMDH_H
#define SIP_SIMDH_H

#include "sip_sim.h"

class Sip_SimDh : public Sip_Sim
{
public:
    Sip_SimDh();
    ~Sip_SimDh();

    virtual bool get_dhpublic_value(unsigned long & dhPublicValueLength, unsigned char * dhPublickValue) = 0;
};

#endif // SIP_SIMDH_H
