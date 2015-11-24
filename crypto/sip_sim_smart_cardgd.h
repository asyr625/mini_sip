#ifndef SIP_SIM_SMART_CARDGD_H
#define SIP_SIM_SMART_CARDGD_H

#include <vector>

#include "smart_card.h"
#include "sip_sim.h"
#include "sip_simdh.h"
#include "sip_simpk.h"

class Sip_Sim_Smart_CardGD : public Smart_Card, public Sip_SimDh, public Sip_SimPk
{
public:
    Sip_Sim_Smart_CardGD();
    ~Sip_Sim_Smart_CardGD();

    bool select_mikey_app();

    bool verify_pin(int verifyMode);
    bool change_pin(const char * newPinCode);

    int is_verified() {return verified_card;}

    bool get_random_value(unsigned char * randomPtr, unsigned long randomLength);

    bool get_signature(unsigned char * dataPtr, int dataLength, unsigned char *signaturePtr, int& signatureLength,
            bool doHash, int hash_alg=HASH_SHA1);

    bool get_key(unsigned char csId, unsigned long csbIdValue,
            unsigned char * randPtr, unsigned long randLength,
            unsigned char * tekPtr, unsigned long tekLength, int keyType);

    bool gen_tgk( unsigned char * dhpubPtr, unsigned long dhpubLength );

    virtual bool get_dhpublic_value(unsigned long & dhPublicValueLength, unsigned char * dhPublickValuePtr);

    bool generate_key_pair();

    bool get_public_key(unsigned char * publicKeyPtr, int keyPairType);
private:
    void clear_buffer();
    int verified_card;
    int user_attempt_timer;
    int admin_attempt_timer;
    int blocked_card;
    unsigned long send_buffer_length;
    unsigned long recv_buffer_length;
    unsigned char * send_buffer;
    unsigned char * recv_buffer;
    unsigned short sw_1_2;
};

#endif // SIP_SIM_SMART_CARDGD_H
