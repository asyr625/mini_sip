#ifndef MIKEY_PAYLOAD_CHASH_H
#define MIKEY_PAYLOAD_CHASH_H

#include "mikey_payload.h"

#define MIKEYPAYLOAD_CHASH_PAYLOAD_TYPE 8

class Mikey_Payload_CHASH : public Mikey_Payload
{
public:
    Mikey_Payload_CHASH(byte_t *start_of_header, int lengthLimit);

    virtual void write_data(byte_t *start, int expectedLength);
    virtual int length();
};

#endif // MIKEY_PAYLOAD_CHASH_H
