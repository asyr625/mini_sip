#include "mikey_payload_cert.h"
#include "mikey_exception.h"
#include "string_utils.h"

#include <string.h>

Mikey_Payload_CERT::Mikey_Payload_CERT( int type, SRef<Certificate *> cert )
{
    this->payload_type_value = MIKEYPAYLOAD_CERT_PAYLOAD_TYPE;
    this->type = type;
    this->cert_length_value = cert->get_der_length();
    this->cert_data_ptr = new byte_t[ cert_length_value ];
    unsigned int size = cert_length_value;
    cert->get_der( this->cert_data_ptr, &size );
}

Mikey_Payload_CERT::Mikey_Payload_CERT( int type, int length, byte_t *data )
{
    this->payload_type_value = MIKEYPAYLOAD_CERT_PAYLOAD_TYPE;
    this->type = type;
    this->cert_length_value = length;
    this->cert_data_ptr = new byte_t[ length ];
    memcpy( this->cert_data_ptr, data, length );
}

Mikey_Payload_CERT::Mikey_Payload_CERT( byte_t * start, int lengthLimit )
    : Mikey_Payload(start)
{
    this->payload_type_value = MIKEYPAYLOAD_CERT_PAYLOAD_TYPE;
    if( lengthLimit < 4 )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a CERT Payload" );
        return;
    }

    set_next_payload_type( start[0] );
    type = start[1];
    cert_length_value = (int)(start[2]) <<8 | start[3];
    if( lengthLimit < 4 + cert_length_value )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a CERT Payload" );
        return;
    }

    cert_data_ptr = new byte_t[ cert_length_value ];
    memcpy( cert_data_ptr, &start[4], cert_length_value );
    end_ptr = start_ptr + 4 + cert_length_value;

    assert( end_ptr - start_ptr == length() );
}

Mikey_Payload_CERT::~Mikey_Payload_CERT()
{
    if( cert_data_ptr )
        delete [] cert_data_ptr;
    cert_data_ptr = NULL;
}

void Mikey_Payload_CERT::write_data( byte_t * start, int expectedLength )
{
    assert( expectedLength == length() );
    memset( start, 0, expectedLength );
    start[0] = next_payload_type();
    start[1] = type;
    start[2] = (byte_t) ((cert_length_value & 0xFF00) >> 8);
    start[3] = (byte_t) (cert_length_value & 0xFF);
    memcpy( &start[4], cert_data_ptr, cert_length_value );
}

int Mikey_Payload_CERT::length()
{
    return 4 + cert_length_value;
}

std::string Mikey_Payload_CERT::debug_dump()
{
    return "Mikey_Payload_CERT: nextPayloadType=<"
            +itoa( next_payload_type() ) +
            "> type=<"+itoa(type) +
            "> length=<" + itoa( cert_length_value )+
            "> data=<" + bin_to_hex( cert_data_ptr, cert_length_value )+ ">";
}

byte_t * Mikey_Payload_CERT::cert_data()
{
    return cert_data_ptr;
}

int Mikey_Payload_CERT::cert_length()
{
    return cert_length_value;
}
