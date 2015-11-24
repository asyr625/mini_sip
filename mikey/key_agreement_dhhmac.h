#ifndef KEY_AGREEMENT_DHHMAC_H
#define KEY_AGREEMENT_DHHMAC_H

#include "my_types.h"
#include "key_agreement_dh.h"
#include "key_agreement_psk.h"

class Key_Agreement_DHHMAC : public Key_Agreement_PSK, public Key_Agreement_DH_Base
{
public:
    Key_Agreement_DHHMAC( const byte_t * psk, int pskLength );
    ~Key_Agreement_DHHMAC();

    int32_t type();

    Mikey_Message* create_message();
};

#endif // KEY_AGREEMENT_DHHMAC_H
