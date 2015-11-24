#ifndef MIKEY_PAYLOAD_RAND_H
#define MIKEY_PAYLOAD_RAND_H

#include "mikey_payload.h"
#include "sip_sim.h"

#define MIKEYPAYLOAD_RAND_PAYLOAD_TYPE 11

class Mikey_Payload_RAND : public Mikey_Payload
{
public:
    Mikey_Payload_RAND();
    Mikey_Payload_RAND( int randlen, byte_t * rand_data );
    Mikey_Payload_RAND( byte_t * start, int lengthLimit );
    Mikey_Payload_RAND(SRef<Sip_Sim *> sim);
    ~Mikey_Payload_RAND();

    virtual int length();
    virtual void write_data( byte_t * start, int expectedLength );
    virtual std::string debug_dump();

    int rand_length();
    byte_t * rand_data();

private:
    int rand_length_value;
    byte_t * rand_data_ptr;
};

#endif // MIKEY_PAYLOAD_RAND_H
