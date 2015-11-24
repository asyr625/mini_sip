#ifndef MIKEY_PAYLOAD_ID_H
#define MIKEY_PAYLOAD_ID_H

#include "mikey_payload.h"

#define MIKEYPAYLOAD_ID_PAYLOAD_TYPE 6

#define MIKEYPAYLOAD_ID_TYPE_NAI 0
#define MIKEYPAYLOAD_ID_TYPE_URI 1

class Mikey_Payload_ID : public Mikey_Payload
{
public:
    Mikey_Payload_ID(int type, int length, byte_t * idData );
    Mikey_Payload_ID( byte_t * start, int lengthLimit );
    ~Mikey_Payload_ID();

    virtual void write_data(byte_t *start, int expectedLength);
    virtual int length();
    virtual std::string debug_dump();

    int id_type() const;
    int id_length() const;
    const byte_t * id_data() const;

private:
    int id_type_value;
    int id_length_value;
    byte_t * id_data_ptr;
};

#endif // MIKEY_PAYLOAD_ID_H
