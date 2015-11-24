#include "sip_sim_soft.h"
#include "rand.h"

Sip_Sim_Soft::Sip_Sim_Soft(SRef<Certificate_Chain*> chain, SRef<Certificate_Set*> cas)
{
    cert_chain = chain;
    ca_set = cas;
}

bool Sip_Sim_Soft::get_signature(unsigned char * data,
        int dataLength,
        unsigned char * signaturePtr,
        int & signatureLength,
        bool doHash,
        int hash_alg)
{
    my_assert(doHash /*we don't support not hashing in SipSimSoft yet...*/);
    my_assert(cert_chain);
    SRef<Certificate*> myCert = cert_chain->get_first();
    my_assert(myCert);
    myCert->sign_data(data, dataLength, signaturePtr, &signatureLength);
    return true;
}

bool Sip_Sim_Soft::get_random_value(unsigned char * randomPtr, unsigned long randomLength)
{
    return Rand::randomize(randomPtr, randomLength);
}
