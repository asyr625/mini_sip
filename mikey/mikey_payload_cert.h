#ifndef MIKEY_PAYLOAD_CERT_H
#define MIKEY_PAYLOAD_CERT_H

#include "sobject.h"
#include "mikey_payload.h"

#define MIKEYPAYLOAD_CERT_PAYLOAD_TYPE 7

#define MIKEYPAYLOAD_CERT_TYPE_X509V3 0
#define MIKEYPAYLOAD_CERT_TYPE_X509V3URL 1
#define MIKEYPAYLOAD_CERT_TYPE_X509V3SIGN 2
#define MIKEYPAYLOAD_CERT_TYPE_X509V3ENCR 3

class Certificate;

class Mikey_Payload_CERT : public Mikey_Payload
{
public:
    Mikey_Payload_CERT( int type, SRef<Certificate *> cert );
    Mikey_Payload_CERT( int type, int length, byte_t *data );
    Mikey_Payload_CERT( byte_t * start, int lengthLimit );
    ~Mikey_Payload_CERT();

    virtual void write_data( byte_t * start, int expectedLength );
    virtual int length();
    virtual std::string debug_dump();

    byte_t * cert_data();
    int cert_length();

private:
    int type;
    int cert_length_value;
    byte_t * cert_data_ptr;
};

#endif // MIKEY_PAYLOAD_CERT_H
