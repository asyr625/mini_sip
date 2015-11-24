#ifndef SIP_SIM_H
#define SIP_SIM_H

#include "sobject.h"
#include "cert.h"

#define HASH_SHA1 1

class Sip_Sim : public SObject
{
public:
    Sip_Sim();
    virtual ~Sip_Sim();

    virtual bool get_signature(unsigned char * data,
                int dataLength,
                unsigned char * signaturePtr,
                int & signatureLength,
                bool doHash,
                int hash_alg = HASH_SHA1) = 0;

    virtual bool get_random_value(unsigned char * randomPtr, unsigned long randomLength) = 0;

    virtual void set_certificate_chain(SRef<Certificate_Chain *> c) { cert_chain = c; }
    virtual SRef<Certificate_Chain *> get_certificate_chain() { return cert_chain; }

    virtual void set_cas(SRef<Certificate_Set*> c) { ca_set = c; }
    virtual SRef<Certificate_Set *> get_cas() { return ca_set; }

protected:
    SRef<Certificate_Chain *> cert_chain;
    SRef<Certificate_Set *> ca_set;
};

#endif // SIP_SIM_H
