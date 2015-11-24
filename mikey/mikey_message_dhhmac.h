#ifndef MIKEY_MESSAGE_DHHMAC_H
#define MIKEY_MESSAGE_DHHMAC_H

#include "mikey_message.h"
#include "key_agreement_dhhmac.h"

class Mikey_Message_DHHMAC : public Mikey_Message
{
public:
    Mikey_Message_DHHMAC();
    Mikey_Message_DHHMAC( Key_Agreement_DHHMAC * ka, int macAlg  = MIKEY_MAC_HMAC_SHA1_160 );

    SRef<Mikey_Message *> parse_response(Key_Agreement  * kaBase );
    void set_offer(Key_Agreement * kaBase );
    SRef<Mikey_Message *> build_response( Key_Agreement * ka );
    bool authenticate(Key_Agreement  * kaBase );

    bool is_initiator_message() const;
    bool is_responder_message() const;
    int32_t key_agreement_type() const;
};

#endif // MIKEY_MESSAGE_DHHMAC_H
