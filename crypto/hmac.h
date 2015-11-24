#ifndef HMAC_H
#define HMAC_H

void hmac_sha1( const unsigned char * key, unsigned int key_length,
                const unsigned char * data, unsigned int data_length,
                unsigned char * mac, unsigned int * mac_length );

void hmac_sha1( const unsigned char * key, unsigned int key_length,
                unsigned char * data[], unsigned int data_length[],
                unsigned char * mac, unsigned int * mac_length );

#endif // HMAC_H
