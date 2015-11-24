#ifndef SIP_SIM_SOFT_H
#define SIP_SIM_SOFT_H

#include "sip_sim.h"

class Sip_Sim_Soft : public Sip_Sim
{
public:
    Sip_Sim_Soft(SRef<Certificate_Chain*> chain, SRef<Certificate_Set*> cas);

    virtual bool get_signature(unsigned char * data,
            int dataLength,
            unsigned char * signaturePtr,
            int & signatureLength,
            bool doHash,
            int hash_alg=HASH_SHA1);

    virtual bool get_random_value(unsigned char * randomPtr, unsigned long randomLength);
};

#endif // SIP_SIM_SOFT_H
