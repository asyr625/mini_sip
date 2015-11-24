#ifndef MIKEY_PAYLOAD_KEMAC_H
#define MIKEY_PAYLOAD_KEMAC_H

#include "mikey_payload.h"
#include "mikey_payload_key_data.h"

#define MIKEYPAYLOAD_KEMAC_PAYLOAD_TYPE 1

#define MIKEY_PAYLOAD_KEMAC_MAC_NULL 0
#define MIKEY_PAYLOAD_KEMAC_MAC_HMAC_SHA1_160 1

#define MIKEY_PAYLOAD_KEMAC_ENCR_NULL       0
#define MIKEY_PAYLOAD_KEMAC_ENCR_AES_CM_128 1
#define MIKEY_PAYLOAD_KEMAC_ENCR_AES_KW_128 2

class Mikey_Payloads;

class Mikey_Payload_KEMAC : public Mikey_Payload
{
public:
    Mikey_Payload_KEMAC( int encr_alg, int encr_dataLength, byte_t * encr_data, int mac_alg, byte_t * mac_data );

    Mikey_Payload_KEMAC( byte_t *start, int lengthLimit );
    ~Mikey_Payload_KEMAC();

    virtual int length();

    virtual void write_data( byte_t *start, int expectedLength );
    virtual std::string debug_dump();

    int encr_alg();
    int encr_data_length();
    byte_t * encr_data();

    Mikey_Payloads* decode_payloads( int firstPayloadType, byte_t * encrKey, int encrKeyLength, byte_t * iv );

    int mac_alg();
    byte_t * mac_data();

    void set_mac( byte_t * data );
private:
    int encr_alg_value;
    int encr_data_length_value;
    byte_t * encr_data_ptr;

    int mac_alg_value;
    byte_t * mac_data_ptr;
};

#endif // MIKEY_PAYLOAD_KEMAC_H
