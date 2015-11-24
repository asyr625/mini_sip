#ifndef ZRTPDH_H
#define ZRTPDH_H

#include "my_types.h"

class ZrtpDH
{
public:
    ZrtpDH(int32_t pkLength);
    ~ZrtpDH();

    int32_t generate_key();
    int32_t get_secret_size() const;
    int32_t get_pub_key_size() const;
    int32_t get_pub_key_bytes(uint8_t *buf) const;
    int32_t compute_key(uint8_t *pubKeyBytes, int32_t length, uint8_t *secret);
    void random(uint8_t *buf, int32_t length) const;
    int32_t check_pub_key(uint8_t *pubKeyBytes, int32_t length) const;

private:
    void *priv;
};

#endif // ZRTPDH_H
