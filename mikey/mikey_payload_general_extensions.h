#ifndef MIKEY_PAYLOAD_GENERAL_EXTENSIONS_H
#define MIKEY_PAYLOAD_GENERAL_EXTENSIONS_H

#include "mikey_payload.h"

#define MIKEYPAYLOAD_GENERALEXTENSIONS_PAYLOAD_TYPE 21

#define MIKEY_EXT_TYPE_VENDOR_ID	0

#define MIKEY_EXT_TYPE_SDP_ID		1

class Mikey_Payload_General_Extensions : public Mikey_Payload
{
public:
    Mikey_Payload_General_Extensions(byte_t *start, int lengthLimit);
    Mikey_Payload_General_Extensions(uint8_t type, uint16_t length, byte_t * data);

    ~Mikey_Payload_General_Extensions();

    virtual void write_data(byte_t *start, int expectedLength);
    virtual int length();
    uint8_t type;
    uint16_t leng;
    byte_t * data;
};

#endif // MIKEY_PAYLOAD_GENERAL_EXTENSIONS_H
