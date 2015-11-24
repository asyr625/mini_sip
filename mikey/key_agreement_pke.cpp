#include "key_agreement_pke.h"
#include "rand.h"

Key_Agreement_PKE::Key_Agreement_PKE( SRef<Certificate_Chain*> cert, SRef<Certificate_Chain*> peerCert )
    : Key_Agreement_PSK(),
      Peer_Certificates(cert, peerCert)
{
    int envKeyLength = 112;

    byte_t *envKey = new byte_t[ envKeyLength ];
    Rand::randomize( envKey, envKeyLength );
    set_psk( envKey, envKeyLength );

    delete []envKey;
    set_v(1);
}

Key_Agreement_PKE::Key_Agreement_PKE( SRef<Certificate_Chain *> cert, SRef<Certificate_Set *> ca_db )
    : Key_Agreement_PSK(),
      Peer_Certificates(cert, ca_db)
{
    int envKeyLength = 112;

    byte_t *envKey = new byte_t[ envKeyLength ];

    Rand::randomize( envKey, envKeyLength );
    set_psk( envKey, envKeyLength );
    delete []envKey;
}

Key_Agreement_PKE::~Key_Agreement_PKE()
{

}

int32_t Key_Agreement_PKE::type()
{
    return KEY_AGREEMENT_TYPE_PK;
}

byte_t* Key_Agreement_PKE::get_envelope_key(void)
{
    return get_psk();
}

int Key_Agreement_PKE::get_envelope_key_length(void)
{
    return get_psk_length();
}

void Key_Agreement_PKE::set_envelope_key( const byte_t *aEnvKey, size_t aEnvKeyLength )
{
    set_psk( aEnvKey, (int)aEnvKeyLength );
}

Mikey_Message* Key_Agreement_PKE::create_message()
{
    return Mikey_Message::create( this );
}
