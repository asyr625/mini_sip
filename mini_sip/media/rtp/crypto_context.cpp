#include "crypto_context.h"

#include "my_assert.h"
#include "mikey_payload_sp.h"

#include<string.h>
#include<iostream>


Crypto_Context::Crypto_Context( uint32_t ssrc )
    : ssrc(ssrc_),
      using_mki(false),mki_length(0),mki(NULL),
      roc(0),guessed_roc(0),s_l(0),key_deriv_rate(0),
      replay_window(0),
      master_key(NULL), master_key_length(0),
      master_key_srtp_use_nb(0), master_key_srtcp_use_nb(0),
      master_salt(NULL), master_salt_length(0),
      n_e(0),k_e(NULL),n_a(0),k_a(NULL),n_s(0),k_s(NULL),
      ealg(MIKEY_SRTP_EALG_NULL), aalg(MIKEY_SRTP_AALG_NULL),
      ekeyl(0), akeyl(0), skeyl(0),
      encr(1), auth(1)
{
}

Crypto_Context::Crypto_Context( uint32_t ssrc, int roc, uint16_t seq_no,
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
                                uint8_t tag_length_ )
    : ssrc(ssrc),using_mki(false),mki_length(0),mki(NULL),
      roc(roc),guessed_roc(0),s_l(seq_no),key_deriv_rate(key_deriv_rate),
      replay_window(0),
      master_key_srtp_use_nb(0), master_key_srtcp_use_nb(0)
{
    this->ealg = ealg;
    this->aalg = aalg;
    this->encr = encr;
    this->auth = auth;
    this->ekeyl = ekeyl;
    this->akeyl = akeyl;
    this->skeyl = skeyl;
    this->master_key_length = master_key_length;
    this->master_key = new unsigned char[ master_key_length ];
    memcpy( this->master_key, master_key, master_key_length );

    this->master_salt_length = master_salt_length;
    this->master_salt = new unsigned char[ master_salt_length ];
    memcpy( this->master_salt, master_salt, master_salt_length );

    switch( ealg )
    {
    case MIKEY_SRTP_EALG_NULL:
        n_e = 0;
        k_e = NULL;
        n_s = 0;
        k_s = NULL;
        break;
    case MIKEY_SRTP_EALG_AESCM:
    case MIKEY_SRTP_EALG_AESF8:
        n_e = ekeyl;
        k_e = new unsigned char[ n_e ];
        n_s = skeyl;
        k_s = new unsigned char[ n_s ];
        break;
    }

    switch( aalg )
    {
    case MIKEY_SRTP_AALG_NULL:
        n_a = 0;
        k_a = NULL;
        this->tag_length = 0;
        break;
    case MIKEY_SRTP_AALG_SHA1HMAC:
        n_a = akeyl;
        k_a = new unsigned char[ n_a ];
        this->tag_length = tag_length_;
        break;
    }
}

Crypto_Context::~Crypto_Context()
{
    if( mki )
        delete [] mki;

    if( master_key )
        delete [] master_key;

    if( master_salt )
        delete [] master_salt;

    if( k_e )
        delete [] k_e;

    if( k_a )
        delete [] k_a;

    if( k_s )
        delete [] k_s;
}

void Crypto_Context::set_roc( unsigned int roc )
{
    this->roc = roc;
}

unsigned int Crypto_Context::get_roc()
{
    return roc;
}

void Crypto_Context::rtp_encrypt( Rtp_Packet * rtp, uint64_t index )
{
    if( ealg == MIKEY_SRTP_EALG_AESCM )
    {

        /* Compute the IV:
          k_s   XX XX XX XX XX XX XX XX XX XX XX XX XX XX
          SSRC              XX XX XX XX
          index                         XX XX XX XX XX XX
          ------------------------------------------------------XOR
          IV    XX XX XX XX XX XX XX XX XX XX XX XX XX XX 00 00
        */

        unsigned char iv[16];
        uint32_t ssrc = rtp->get_header().get_ssrc();
        memcpy( iv, k_s, 4 );

        int i;
        for(i = 4; i < 8; i++ )
        {
            iv[i] = ( 0xFF & ( ssrc >> ((7-i)*8) ) ) ^ k_s[i];
        }

        for(i = 8; i < 14; i++ )
        {
            iv[i] = ( 0xFF & (unsigned char)( index >> ((13-i)*8) ) ) ^ k_s[i];
        }

        iv[14] = iv[15] = 0;

        AES * aes = new AES( k_e, n_e );
        aes->ctr_encrypt( rtp->get_content(),
                          rtp->get_content_length(),
                          iv );
        delete aes;
    }

    if( ealg == MIKEY_SRTP_EALG_AESF8 )
    {

        /* Create the IV:
           IV = 0x00 || M || PT || SEQ  ||      TS    ||    SSRC   ||    ROC
                8Bit  1bit  7bit  16bit       32bit        32bit        32bit
           ------------\       /----------------------------
                 XX       XX      XX XX   XX XX XX XX   XX XX XX XX  XX XX XX XX
        */
        unsigned char iv[16];
        Rtp_Header &rtph = rtp->get_header();
        uint16_t seq_no = rtph.get_seq_no();
        uint32_t ui32;
        uint32_t *ui32p = (uint32_t *)iv;
        uint16_t *ui16p = (uint16_t *)iv;

        iv[0] = 0;
        iv[1] = rtph.get_marker() ? 0x80 : 0x00;
        iv[1] |= rtph.get_payload_type() & 0x7f;
        ui16p[1] = hton16(seq_no);


        // Set the timestamp in big endian order into IV
        ui32 = rtph.get_timestamp();
        ui32p[1] = hton32(ui32);

        // set the SSRC in big endian order into IV
        ui32 = rtph.get_ssrc();
        ui32p[2] = hton32(ui32);

        // set the ROC in big endian order into IV
        // TODO: cross check if ROC has to be in big endian order and if we have
        // the correct (current) ROC here !!!

        ui32p[3] = hton32(roc);

        AES *aes = new AES(k_e, n_e);
        aes->f8_encrypt(rtp->get_content(),
                        rtp->get_content_length(),
                        iv, k_e, n_e, k_s, n_s);
        delete aes;
    }
}

void Crypto_Context::rtp_authenticate( Rtp_Packet * rtp, uint32_t roc, unsigned char * tag )
{
    unsigned int tag_length;

    if( aalg == MIKEY_SRTP_AALG_SHA1HMAC )
    {
        unsigned char temp[20];
        unsigned char * chunks[6];
        unsigned int chunkLength[6];
        uint32_t beRoc = hton32( roc );

        char* bytes = rtp->get_header().get_bytes();
        unsigned char* content = rtp->get_content();
        unsigned char* extension = rtp->get_extension_header();

        unsigned int header_size = rtp->get_header().size() ;
        unsigned int content_size = rtp->get_content_length() ;
        unsigned int extensionSize = rtp->get_extension_length();

        int offset = 0;
        chunks[offset] = (unsigned char *)bytes;
        chunkLength[offset++] = header_size;
        if (extension != NULL)
        {
            chunks[offset] = extension;
            chunkLength[offset++] = extensionSize;
        }
        if (content != NULL)
        {
            chunks[offset] = (unsigned char *)content;
            chunkLength[offset++] = content_size;
        }
        // TODO check regarding MIKEY
        chunks[offset] = (unsigned char *)&beRoc;
        chunkLength[offset++] = 4;
        chunks[offset] = NULL;

        hmac_sha1( k_a, n_a,
                   /* data */ chunks,
                   /*authenticated part length */
                   chunkLength,
                   /* tag */  temp, &tag_length );
        massert( tag_length == 20 );
        /* truncate the result */
        memcpy( tag, temp, get_tag_length() );
        delete [] bytes;

    }
}

/* used by the key derivation method */
static void compute_iv( unsigned char * iv, uint64_t label, uint64_t index,
                        int64_t key_deriv_rate, unsigned char * master_salt )
{

    uint64_t key_id;

    if( key_deriv_rate == 0 )
        key_id = label << 48;
    else
        key_id = ((label << 48) | (index / key_deriv_rate));

    //printf( "Key_ID: %llx\n", key_id );

    /* compute the IV
        key_id:                           XX XX XX XX XX XX XX
        master_salt: XX XX XX XX XX XX XX XX XX XX XX XX XX XX
        ------------------------------------------------------------ XOR
        IV:          XX XX XX XX XX XX XX XX XX XX XX XX XX XX 00 00
        */

    int i;
    for(i = 0; i < 7 ; i++ ){
        iv[i] = master_salt[i];
    }

    for(i = 7; i < 14 ; i++ ){
        iv[i] = (unsigned char)(0xFF & (key_id >> (8*(13-i)))) ^
                master_salt[i];
    }

    iv[14] = iv[15] = 0;
}

void Crypto_Context::derive_srtp_keys( uint64_t index )
{
    AES * aes;
    unsigned char iv[16];

    // Compute session encryption key
    uint64_t label = 0;
    compute_iv( iv, label, index, key_deriv_rate, master_salt );
    aes = new AES( master_key, master_key_length );
    aes->get_ctr_cipher_stream( k_e, n_e, iv );
    delete aes;

    // Compute session authentication key
    label = 0x01;
    compute_iv( iv, label, index, key_deriv_rate, master_salt );
    aes = new AES( master_key, master_key_length );
    aes->get_ctr_cipher_stream( k_a, n_a, iv );
    delete aes;

    // Compute session salt
    label = 0x02;
    compute_iv( iv, label, index, key_deriv_rate, master_salt );
    aes = new AES( master_key, master_key_length );
    aes->get_ctr_cipher_stream( k_s, n_s, iv );
    delete aes;
}

uint64_t Crypto_Context::guess_index( unsigned short new_seq_nb )
{
    if( s_l < 32768 ){
        if( new_seq_nb - s_l > 32768 )
            guessed_roc = roc - 1;
        else
            guessed_roc = roc;
    }
    else{
        if( s_l - 32768 > new_seq_nb )
            guessed_roc = roc + 1;
        else
            guessed_roc = roc;
    }

    return ((uint64_t)guessed_roc) << 16 | new_seq_nb;
}

bool Crypto_Context::check_replay( unsigned short new_seq_nb )
{
    if( aalg == MIKEY_SRTP_AALG_NULL && ealg == MIKEY_SRTP_EALG_NULL )
    {
        /* No security policy, don't use the replay protection */
        return true;
    }

    uint64_t guessed_index = guess_index( new_seq_nb );
    uint64_t local_index = (((uint64_t)roc) << 16 & 0xFFFF) | s_l;

    int64_t delta = guessed_index - local_index;
    //fprintf( stderr, "roc: %u\n", roc );
    //fprintf( stderr, "guessed_index: %llu\n", guessed_index );
    //fprintf( stderr, "local_index: %llu\n", local_index );
    //fprintf( stderr, "s_l: %lli\n", s_l );


    //fprintf( stderr, "delta: %lli\n", delta );
    if( delta > 0 )
        /* Packet not yet received*/
        return true;
    else
    {
        if( -delta > REPLAY_WINDOW_SIZE )
            /* Packet too old */
            return false;
        else
        {
            if( (replay_window >> (-delta)) & 0x1 )
                /* Packet already received ! */
                return false;
            else
                /* Packet not yet received */
                return true;
        }
    }
}

void Crypto_Context::update( unsigned short new_seq_nb )
{
    int64_t delta = guess_index( new_seq_nb ) -
            ( ((uint64_t)roc) << 16 | s_l );

    /* update the replay bitmask */
    if( delta > 0 )
    {
        replay_window = replay_window << delta;
        replay_window |= 1;
    }
    else{
        replay_window |= ( 1 << delta );
    }

    /* update the locally stored ROC and highest sequence number */
    if( new_seq_nb > s_l )
        s_l = new_seq_nb;
    if( guessed_roc > roc )
    {
        roc = guessed_roc;
        s_l = new_seq_nb;
    }
}

int Crypto_Context::get_tag_length()
{
    switch( aalg )
    {
    case MIKEY_SRTP_AALG_NULL:
        return 0;
    case MIKEY_SRTP_AALG_SHA1HMAC:
        return tag_length;
    }
    return -1;
}

int Crypto_Context::get_mki_length()
{
    return mki_length;
}

Crypto_Context* Crypto_Context::new_crypto_context_for_ssrc(uint32_t ssrc, int roc, uint16_t seq, int64_t keyDerivRate)
{
    Crypto_Context* pcc = new Crypto_Context(
                ssrc,
                roc,                                     // Roll over Counter,
                seq,
                keyDerivRate,                            // keyderivation << 48,
                this->ealg,                              // encryption algo
                this->aalg,                              // authentication algo
                this->master_key,                        // Master Key
                this->master_key_length,                 // Master Key length
                this->master_salt,                       // Master Salt
                this->master_salt_length,                // Master Salt length
                this->ekeyl,                             // encryption keyl
                this->akeyl,                             // authentication key len
                this->skeyl,                             // session salt len
                this->encr,
                this->auth,
                this->tag_length);                        // authentication tag len
    return pcc;
}
