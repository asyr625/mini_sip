#ifndef MIKEY_PAYLOAD_DH_H
#define MIKEY_PAYLOAD_DH_H

#include "mikey_payload.h"

#include "key_validity.h"

#define MIKEYPAYLOAD_DH_PAYLOAD_TYPE 3

#define MIKEYPAYLOAD_DH_GROUP5 0
#define MIKEYPAYLOAD_DH_GROUP1 1
#define MIKEYPAYLOAD_DH_GROUP2 2


class Mikey_Payload_DH : public Mikey_Payload
{
public:
    Mikey_Payload_DH( int dhGroup, byte_t * dhKey, SRef<Key_Validity *> kv );
    Mikey_Payload_DH( byte_t * start, int lengthLimit );
    ~Mikey_Payload_DH();

    virtual void write_data( byte_t * start, int expectedLength );
    virtual int length();
    virtual std::string debug_dump();

    int group();
    byte_t * dh_key();
    int dh_key_length();

    SRef<Key_Validity *> kv();

private:
    int dh_group;
    int dh_key_length_value;
    byte_t * dh_key_ptr;
    SRef<Key_Validity *> kv_ptr;
};

#endif // MIKEY_PAYLOAD_DH_H
