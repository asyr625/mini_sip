#ifndef KEY_AGREEMENT_PSK_H
#define KEY_AGREEMENT_PSK_H

#include "key_agreement.h"

class Key_Agreement_PSK : public Key_Agreement
{
public:
    Key_Agreement_PSK( const byte_t * psk, int pskLength );
    virtual ~Key_Agreement_PSK();

    int32_t type();

    void generate_tgk( uint32_t tgkLength = 192 );
    void gen_transp_encr_key( byte_t * encrKey, int encrKeyLength );
    void gen_transp_salt_key(byte_t * saltKey, int encrKeyLength );
    void gen_transp_auth_key( byte_t * authKey, int authKeyLength );
    uint64_t tsent();
    void set_tsent( uint64_t tSent );
    uint64_t t_received;
    byte_t * auth_key;
    unsigned int auth_key_length;
    void set_v(int value) { v = value; }
    int get_v() { return v; }

    int mac_alg;

    virtual Mikey_Message* create_message();

protected:
    Key_Agreement_PSK();
    void set_psk( const byte_t* psk, int pskLength );
    byte_t* get_psk();
    int get_psk_length();

private:
    byte_t * psk_ptr;
    int psk_length_value;

    /**
     * The V bit
     */
    int v;

    /**
     * Timestamp from when the message was sent
     */
    uint64_t tsent_value;
};

#endif // KEY_AGREEMENT_PSK_H
