#ifndef MIKEY_MESSAGE_PSK_H
#define MIKEY_MESSAGE_PSK_H

#include "mikey_message.h"
#include "key_agreement_psk.h"

class Mikey_Message_PSK : public Mikey_Message
{
public:
    Mikey_Message_PSK();
    Mikey_Message_PSK( Key_Agreement_PSK * ka, int encrAlg = MIKEY_ENCR_AES_CM_128, int macAlg  = MIKEY_MAC_HMAC_SHA1_160 );

    SRef<Mikey_Message *> parse_response(Key_Agreement  * kaBase );
    void set_offer(Key_Agreement * kaBase );
    SRef<Mikey_Message *> build_response(Key_Agreement * kaBase );
    bool authenticate(Key_Agreement  * kaBase );

    bool is_initiator_message() const;
    bool is_responder_message() const;
    int32_t key_agreement_type() const;
};

#endif // MIKEY_MESSAGE_PSK_H
