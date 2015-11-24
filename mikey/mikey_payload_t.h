#ifndef MIKEY_PAYLOAD_T_H
#define MIKEY_PAYLOAD_T_H

#include "mikey_payload.h"

#define MIKEYPAYLOAD_T_PAYLOAD_TYPE 5

#define T_TYPE_NTP_UTC 0
#define T_TYPE_NTP     1
#define T_TYPE_COUNTER 2

#define NTP_EPOCH_OFFSET 2208988800UL

class Mikey_Payload_T : public Mikey_Payload
{
public:
    Mikey_Payload_T();
    Mikey_Payload_T( int type, uint64_t value );
    Mikey_Payload_T( byte_t * start, int lengthLimit );
    ~Mikey_Payload_T();

    virtual int length();

    virtual void write_data( byte_t *start, int expectedLength );
    virtual std::string debug_dump();

    int64_t offset( int type, uint64_t ts );
    bool check_offset( uint64_t max );

    uint64_t ts();

private:
    int ts_type_value;
    uint64_t ts_value;
};

#endif // MIKEY_PAYLOAD_T_H
