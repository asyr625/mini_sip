#ifndef SHA256_H
#define SHA256_H

void sha256(unsigned char *data, unsigned int data_length, unsigned char *digest);

void sha256(unsigned char *data[], unsigned int data_length[], unsigned char *digest);

#endif // SHA256_H
