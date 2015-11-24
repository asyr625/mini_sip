#ifndef CRYPTO_CONTEXT_H
#define CRYPTO_CONTEXT_H

#include "sobject.h"
#include "aes.h"
#include "hmac.h"

class Rtp_Packet;

class Crypto_Context : public SObject
{
public:
    Crypto_Context( uint32_t ssrc );

    Crypto_Context(uint32_t ssrc, int roc, uint16_t seq_no,
                    int64_t key_deriv_rate,
                    //enum encr_method encryption,
                    uint8_t ealg,
                    //enum auth_method authentication,
                    uint8_t aalg,
                    unsigned char * master_key,
                    unsigned int master_key_length,
                    unsigned char * master_salt,
                    unsigned int master_salt_length,
                    uint8_t ekeyl,
                    uint8_t akeyl,
                    uint8_t skeyl,
                    uint8_t encr,
                    uint8_t auth,
                    uint8_t tag_length_ );
    ~Crypto_Context();

    void set_roc( unsigned int roc );
    unsigned int get_roc();

    void rtp_encrypt( Rtp_Packet * rtp, uint64_t index );
    void rtp_authenticate( Rtp_Packet * rtp, uint32_t roc, unsigned char * tag );
    void derive_srtp_keys( uint64_t index );
    uint64_t guess_index( unsigned short new_seq_nb );
    bool check_replay( unsigned short new_seq_nb );
    void update( unsigned short new_seq_nb );

    int get_tag_length();
    int get_mki_length();

    virtual std::string get_mem_object_type() const  { return "CryptoContext"; }

    uint32_t get_ssrc() { return ssrc; }

    uint8_t get_ealg()  { return ealg; }

    Crypto_Context* new_crypto_context_for_ssrc(uint32_t ssrc, int roc, uint16_t seq, int64_t keyDerivRate);
private:

    uint32_t ssrc;
    bool using_mki;
    unsigned int mki_length;
    unsigned char * mki;

    unsigned int roc;
    unsigned int guessed_roc;
    unsigned short s_l;
    int64_t key_deriv_rate;

    /* bitmask for replay check */
    uint64_t replay_window;

    unsigned char * master_key;
    unsigned int master_key_length;
    unsigned int master_key_srtp_use_nb;
    unsigned int master_key_srtcp_use_nb;
    unsigned char * master_salt;
    unsigned int master_salt_length;

    /* Session Encryption, Authentication keys, Salt */
    int n_e;
    unsigned char * k_e;
    int n_a;
    unsigned char * k_a;
    int n_s;
    unsigned char * k_s;

    //enum encr_method encryption;
    //enum auth_method authentication;
    uint8_t ealg;
    uint8_t aalg;
    uint8_t ekeyl;
    uint8_t akeyl;
    uint8_t skeyl;
    uint8_t encr;
    uint8_t auth;
    uint8_t tag_length;
};

#endif // CRYPTO_CONTEXT_H
