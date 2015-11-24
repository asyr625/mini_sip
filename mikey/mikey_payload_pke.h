#ifndef MIKEY_PAYLOAD_PKE_H
#define MIKEY_PAYLOAD_PKE_H

#include "mikey_payload.h"

#define MIKEYPAYLOAD_PKE_PAYLOAD_TYPE 2

class Mikey_Payload_PKE : public Mikey_Payload
{
public:
    Mikey_Payload_PKE( int c, byte_t * data, int data_length );

    Mikey_Payload_PKE( byte_t * start, int lengthLimit );
    ~Mikey_Payload_PKE();

    virtual int length();

    virtual void write_data( byte_t * start, int expectedLength );
    virtual std::string debug_dump();
    int c();
    int data_length() const;
    const byte_t * data() const;

private:
    int cvalue;
    int data_length_value;
    byte_t * data_ptr;
};

#endif // MIKEY_PAYLOAD_PKE_H
