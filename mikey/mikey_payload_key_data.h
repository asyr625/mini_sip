#ifndef MIKEY_PAYLOAD_KEY_DATA_H
#define MIKEY_PAYLOAD_KEY_DATA_H

#include "mikey_payload.h"
#include "key_validity.h"

#define MIKEYPAYLOAD_KEYDATA_PAYLOAD_TYPE 20

#define KEYDATA_TYPE_TGK      0
#define KEYDATA_TYPE_TGK_SALT 1
#define KEYDATA_TYPE_TEK      2
#define KEYDATA_TYPE_TEK_SALT 3

class Mikey_Payload_Key_Data : public Mikey_Payload
{
public:
    Mikey_Payload_Key_Data( int type, byte_t * keyData, int keyDataLength, SRef<Key_Validity *> kv );
    Mikey_Payload_Key_Data( int type, byte_t * keyData, int keyDataLength, byte_t * saltData, int saltDataLength, SRef<Key_Validity *> kv );
    Mikey_Payload_Key_Data( byte_t * start, int lengthLimit );
    ~Mikey_Payload_Key_Data();

    virtual void write_data( byte_t * start, int expectedLength );
    virtual int length();
    virtual std::string debug_dump();

    int type();
    SRef<Key_Validity *> kv();

    byte_t * key_data();
    int key_data_length();

    byte_t * salt_data();
    int salt_data_length();

private:
    int type_value;

    byte_t * key_data_ptr;
    int key_data_length_value;

    byte_t * salt_data_ptr;
    int salt_data_length_value;

    SRef<Key_Validity *> kv_ptr;
};

#endif // MIKEY_PAYLOAD_KEY_DATA_H
