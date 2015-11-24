#include "key_agreement_dhhmac.h"

Key_Agreement_DHHMAC::Key_Agreement_DHHMAC( const byte_t * psk, int pskLength )
    : Key_Agreement_PSK(psk, pskLength),
      Key_Agreement_DH_Base(NULL)
{
}

Key_Agreement_DHHMAC::~Key_Agreement_DHHMAC()
{
}

int32_t Key_Agreement_DHHMAC::type()
{
    return KEY_AGREEMENT_TYPE_DHHMAC;
}

Mikey_Message* Key_Agreement_DHHMAC::create_message()
{
    return Mikey_Message::create( this );
}
