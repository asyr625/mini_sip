#ifndef MIKEY_PAYLOAD_SIGN_H
#define MIKEY_PAYLOAD_SIGN_H

#include "mikey_payload.h"

#define MIKEYPAYLOAD_SIGN_PAYLOAD_TYPE 4

#define MIKEYPAYLOAD_SIGN_TYPE_RSA_PKCS 0
#define MIKEYPAYLOAD_SIGN_TYPE_RSA_PSS 1

class Mikey_Payload_SIGN : public Mikey_Payload
{
public:
    Mikey_Payload_SIGN( int sigLength, int type );
    Mikey_Payload_SIGN( byte_t * start, int lengthLimit );
    ~Mikey_Payload_SIGN();

    virtual void write_data( byte_t *start, int expectedLength );
    virtual int length();
    virtual std::string debug_dump();

    int sig_length();
    byte_t * sig_data();
    int sig_type();

    void set_sig_data( byte_t * data, int sigLength);

private:
    int sig_type_value;
    int sig_length_value;
    byte_t * sig_data_ptr;
};

#endif // MIKEY_PAYLOAD_SIGN_H
