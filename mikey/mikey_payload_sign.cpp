#include "mikey_payload_sign.h"
#include "mikey_exception.h"
#include "string_utils.h"

Mikey_Payload_SIGN::Mikey_Payload_SIGN( int sigLength, int type )
{
    this->payload_type_value = MIKEYPAYLOAD_SIGN_PAYLOAD_TYPE;
    set_next_payload_type( 0 ); // Since no other payload will set
    // this value
    this->sig_length_value = sigLength;
    this->sig_data_ptr = new byte_t[ sigLength ];
    memset( this->sig_data_ptr, 0, sigLength );
    this->sig_type_value = type;
}

Mikey_Payload_SIGN::Mikey_Payload_SIGN( byte_t * start, int lengthLimit )
    : Mikey_Payload( start )
{
    if( lengthLimit < 2 )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a SIGN Payload" );
        return;
    }
    this->payload_type_value = MIKEYPAYLOAD_SIGN_PAYLOAD_TYPE;
    set_next_payload_type( 0 ); // Always last payload
    sig_type_value = (start[0] >> 4 )&0x0F;
    sig_length_value = ((int)(start[0]&0x0F)) << 8 | start[1];
    if( lengthLimit < 2 + sig_length_value )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a SIGN Payload" );
        return;
    }
    sig_data_ptr = new byte_t[ sig_length_value ];
    memcpy( sig_data_ptr, &start[2], sig_length_value );
    end_ptr = start_ptr + 2 + sig_length_value;

    assert( end_ptr - start_ptr == length() );
}

Mikey_Payload_SIGN::~Mikey_Payload_SIGN()
{
    if( sig_data_ptr )
        delete [] sig_data_ptr;
    sig_data_ptr = NULL;
}

void Mikey_Payload_SIGN::write_data( byte_t *start, int expectedLength )
{
    assert( expectedLength == length() );
    memset( start, 0, expectedLength );
    start[0] = (byte_t)((( sig_length_value & 0x0F00 ) >> 8) | ((sig_type_value << 4) & 0xF0)) ;
    start[1] = (byte_t)( sig_length_value & 0xFF );

    memcpy( &start[2], sig_data_ptr, sig_length_value );
}

int Mikey_Payload_SIGN::length()
{
    return 2 + sig_length_value;
}

std::string Mikey_Payload_SIGN::debug_dump()
{
    return "Mikey_Payload_SIGN: type=<"+itoa(sig_type_value)+"> length=<"
            +itoa(sig_length_value)+"> signature=<"+bin_to_hex( sig_data_ptr, sig_length_value )+">";
}

int Mikey_Payload_SIGN::sig_length()
{
    return sig_length_value;
}

byte_t * Mikey_Payload_SIGN::sig_data()
{
    return sig_data_ptr;
}

int Mikey_Payload_SIGN::sig_type()
{
    return sig_type_value;
}

void Mikey_Payload_SIGN::set_sig_data( byte_t * data, int sigLength)
{
    if( sig_data_ptr )
        delete [] sig_data_ptr;

    sig_length_value = sigLength;
    sig_data_ptr = new byte_t[ sig_length_value ];
    memcpy( sig_data_ptr, data, sig_length_value );
}
