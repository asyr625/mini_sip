#ifndef MIKEY_PAYLOAD_V_H
#define MIKEY_PAYLOAD_V_H

#include "mikey_payload.h"

#define MIKEYPAYLOAD_V_PAYLOAD_TYPE 9

#define MIKEY_PAYLOAD_V_MAC_NULL 0
#define MIKEY_PAYLOAD_V_MAC_HMAC_SHA1_160 1

class Mikey_Payload_V : public Mikey_Payload
{
public:
    Mikey_Payload_V( int mac_alg, byte_t * verData );
    Mikey_Payload_V( byte_t * start, int lengthLimit );
    ~Mikey_Payload_V();

    virtual void write_data( byte_t * start, int expectedLength );
    virtual int length();

    int mac_alg();
    byte_t * ver_data();

    void set_mac( byte_t * data );

private:
    int mac_alg_value;
    byte_t * ver_data_ptr;
};

#endif // MIKEY_PAYLOAD_V_H
