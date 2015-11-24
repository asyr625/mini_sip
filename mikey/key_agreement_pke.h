#ifndef KEY_AGREEMENT_PKE_H
#define KEY_AGREEMENT_PKE_H

#include "key_agreement.h"
#include "key_agreement_psk.h"
#include "key_agreement_dh.h"
#include "cert.h"

class Key_Agreement_PKE : public Key_Agreement_PSK, public Peer_Certificates
{
public:
    Key_Agreement_PKE( SRef<Certificate_Chain*> cert, SRef<Certificate_Chain*> peerCert );

    Key_Agreement_PKE( SRef<Certificate_Chain *> cert, SRef<Certificate_Set *> ca_db );

    ~Key_Agreement_PKE();

    int32_t type();
    byte_t* get_envelope_key(void);

    int get_envelope_key_length(void);

    void set_envelope_key( const byte_t *aEnvKey, size_t aEnvKeyLength );

    Mikey_Message* create_message();
};

#endif // KEY_AGREEMENT_PKE_H
