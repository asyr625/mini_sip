#ifndef MIKEY_PAYLOAD_H
#define MIKEY_PAYLOAD_H

#include "sobject.h"
#include "my_types.h"

#include<string.h>

#define MIKEYPAYLOAD_LAST_PAYLOAD 0

class Mikey_Payload : public SObject
{
public:
    static const int Last_Payload;

    Mikey_Payload();

    Mikey_Payload( byte_t *start_of_message );
    virtual ~Mikey_Payload();

    int next_payload_type();

    void set_next_payload_type(int t);

    int payload_type();

    byte_t * end();

    virtual int length() = 0;

    virtual void write_data(byte_t *start, int expectedLength) = 0;

    virtual std::string debug_dump() { return "not_defined"; }
protected:
    bool raw_packet_valid;

    byte_t *start_ptr;
    byte_t * end_ptr;

    int next_payload_type_value;
    int payload_type_value;
};

#endif // MIKEY_PAYLOAD_H
