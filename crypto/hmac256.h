#ifndef HMAC256_H
#define HMAC256_H

void hmac_sha256( unsigned char * key, unsigned int key_length,
                unsigned char * data, unsigned int data_length,
                unsigned char * mac, unsigned int * mac_length );

void hmac_sha256( unsigned char * key, unsigned int key_length,
                unsigned char * data[], unsigned int data_length[],
                unsigned char * mac, unsigned int * mac_length );

#endif // HMAC256_H
