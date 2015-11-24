#include "key_agreement_psk.h"
#include <string.h>

Key_Agreement_PSK::Key_Agreement_PSK()
    : Key_Agreement(),t_received(0), auth_key(NULL), auth_key_length(0),
      mac_alg(0), psk_ptr(NULL), psk_length_value(0),v(0), tsent_value(0)
{
}


Key_Agreement_PSK::Key_Agreement_PSK( const byte_t * psk, int pskLength )
    : Key_Agreement(),t_received(0),auth_key(NULL),auth_key_length(0),
      mac_alg(0),psk_ptr(NULL),psk_length_value(0),v(0),tsent_value(0)
{
    this->psk_length_value = pskLength;
    this->psk_ptr = new unsigned char[ pskLength ];
    memcpy( this->psk_ptr, psk, pskLength );
}

Key_Agreement_PSK::~Key_Agreement_PSK()
{
    if( psk_ptr )
    {
        delete [] psk_ptr;
    }

    if( auth_key )
    {
        delete[] auth_key;
        auth_key = NULL;
    }
}

int32_t Key_Agreement_PSK::type()
{
    return KEY_AGREEMENT_TYPE_PSK;
}

void Key_Agreement_PSK::generate_tgk( uint32_t tgkLength )
{
    set_tgk( NULL, tgkLength );
}

void Key_Agreement_PSK::gen_transp_encr_key( byte_t * encrKey, int encrKeyLength )
{
    key_deriv( 0xFF, csb_id(), psk_ptr, psk_length_value,
            encrKey, encrKeyLength, KEY_DERIV_TRANS_ENCR );
}

void Key_Agreement_PSK::gen_transp_salt_key( byte_t * encrKey, int encrKeyLength )
{
    key_deriv( 0xFF, csb_id(), psk_ptr, psk_length_value,
            encrKey, encrKeyLength, KEY_DERIV_TRANS_SALT );
}

void Key_Agreement_PSK::gen_transp_auth_key( byte_t * authKey, int authKeyLength )
{
    key_deriv( 0xFF, csb_id(), psk_ptr, psk_length_value,
            authKey, authKeyLength, KEY_DERIV_TRANS_AUTH );
}

uint64_t Key_Agreement_PSK::tsent()
{
    return tsent_value;
}

void Key_Agreement_PSK::set_tsent( uint64_t tSent )
{
    this->tsent_value = tSent;
}

Mikey_Message* Key_Agreement_PSK::create_message()
{
    return Mikey_Message::create( this );
}

void Key_Agreement_PSK::set_psk( const byte_t* psk, int pskLength )
{
    if( psk_ptr )
    {
        delete[] psk_ptr;
        psk_ptr = NULL;
    }

    psk_length_value = pskLength;
    psk_ptr = new byte_t[ pskLength ];
    memcpy( psk_ptr, psk, pskLength );
}

byte_t* Key_Agreement_PSK::get_psk()
{
    return psk_ptr;
}

int Key_Agreement_PSK::get_psk_length()
{
    return psk_length_value;
}
