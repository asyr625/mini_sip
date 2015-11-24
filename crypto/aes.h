#ifndef AES_H
#define AES_H

#include "my_types.h"
#include <stdlib.h>

typedef struct _f8_ctx {
    unsigned char *S;
    unsigned char *ivAccent;
    uint32_t J;
} F8_CIPHER_CTX;

class AES
{
public:
    AES( unsigned char * key, int key_length );
    ~AES();

    void encrypt( const unsigned char * input, unsigned char * output );
    void get_ctr_cipher_stream( unsigned char * output, unsigned int length,
            unsigned char * iv );
    /* Counter-mode encryption */
    void ctr_encrypt( const unsigned char * input,
              unsigned int input_length,
         unsigned char * output, unsigned char * iv );

    /* Counter-mode encryption, in place */
    void ctr_encrypt( unsigned char * data,
              unsigned int data_length,
              unsigned char * iv );

    /* f8-mode encryption, in place */
    void f8_encrypt( unsigned char * data,
             unsigned int data_length,
             unsigned char * iv,
             unsigned char *key,
             unsigned int keyLen,
             unsigned char *salt,
             unsigned int saltLen);

    /* f8-mode encryption */
    void f8_encrypt(unsigned char *in,
            unsigned int in_length,
            unsigned char *out,
            unsigned char *iv,
            unsigned char *key,
            unsigned int keyLen,
            unsigned char *salt,
            unsigned int saltLen);


private:
    void set_encrypt_key( unsigned char *key, int key_nb_bits );

    int process_block(F8_CIPHER_CTX *f8ctx,
             unsigned char *in,
             int length,
             unsigned char *out);
    void *m_key;

    AES();
};

#endif // AES_H
