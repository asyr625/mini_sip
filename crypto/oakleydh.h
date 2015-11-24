#ifndef OAKLEYDH_H
#define OAKLEYDH_H

#include "my_types.h"

class OakleyDH
{
public:
    OakleyDH();
    OakleyDH( int group );
    ~OakleyDH();

    bool set_group( int group );
    int group() const;

    int compute_secret(const uint8_t *peerKey,
              uint32_t peerKeyLenght,
              uint8_t *secret,
              uint32_t secretLength) const;

    /** Length of public key in bytes  */
    uint32_t public_key_length() const;
    uint32_t get_public_key( byte_t *buf, uint32_t buflen) const;

    /** Length of secret in bytes  */
    uint32_t secret_length() const;

protected:
    bool generate_key();

private:
    int group_value;
    void * priv;
};

#endif // OAKLEYDH_H
