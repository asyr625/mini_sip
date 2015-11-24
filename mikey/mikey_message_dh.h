#ifndef MIKEY_MESSAGE_DH_H
#define MIKEY_MESSAGE_DH_H

#include "mikey_message.h"
#include "key_agreement_dh.h"

class Mikey_Message_DH : public Mikey_Message
{
public:
    Mikey_Message_DH();
    Mikey_Message_DH( Key_Agreement_DH * ka );

    SRef<Mikey_Message *> parse_response(Key_Agreement  * kaBase );
    void set_offer(Key_Agreement * kaBase );
    SRef<Mikey_Message *> build_response( Key_Agreement * ka );
    bool authenticate(Key_Agreement  * kaBase );

    bool is_initiator_message() const;
    bool is_responder_message() const;
    int32_t key_agreement_type() const;
};

#endif // MIKEY_MESSAGE_DH_H
