#ifndef MIKEY_PAYLOAD_ERR_H
#define MIKEY_PAYLOAD_ERR_H

#include "mikey_payload.h"

#define MIKEYPAYLOAD_ERR_PAYLOAD_TYPE 12

#define MIKEY_ERR_TYPE_AUTH_FAILURE  0
#define MIKEY_ERR_TYPE_INVALID_TS    1
#define MIKEY_ERR_TYPE_INVALID_PRF   2
#define MIKEY_ERR_TYPE_INVALID_MAC   3
#define MIKEY_ERR_TYPE_INVALID_EA    4
#define MIKEY_ERR_TYPE_INVALID_HA    5
#define MIKEY_ERR_TYPE_INVALID_DH    6
#define MIKEY_ERR_TYPE_INVALID_ID    7
#define MIKEY_ERR_TYPE_INVALID_CERT  8
#define MIKEY_ERR_TYPE_INVALID_SP    9
#define MIKEY_ERR_TYPE_INVALID_SPPAR 10
#define MIKEY_ERR_TYPE_INVALID_DT    11
#define MIKEY_ERR_TYPE_UNSPEC        12

class Mikey_Payload_ERR : public Mikey_Payload
{
public:
    Mikey_Payload_ERR( int errType );
    Mikey_Payload_ERR( byte_t * start, int lengthLimit );
    ~Mikey_Payload_ERR();

    virtual void write_data( byte_t * start, int expectedLength );
    virtual int length();
    virtual std::string debug_dump();

    int error_type();

private:
    int err_type_value;
};

#endif // MIKEY_PAYLOAD_ERR_H
