#include "mikey_payload_pke.h"
#include "mikey_exception.h"
#include "string_utils.h"

Mikey_Payload_PKE::Mikey_Payload_PKE( int c, byte_t * data, int data_length )
{
    this->payload_type_value = MIKEYPAYLOAD_PKE_PAYLOAD_TYPE;
    this->cvalue = c;
    this->data_length_value = data_length;
    this->data_ptr = new byte_t[ data_length ];
    memcpy( this->data_ptr, data, data_length );
}

Mikey_Payload_PKE::Mikey_Payload_PKE( byte_t * start, int lengthLimit )
    : Mikey_Payload(start)
{
    if(  lengthLimit < 3 )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given dataPtr is too short to form a PKE Payload" );
        return;
    }
    this->payload_type_value = MIKEYPAYLOAD_PKE_PAYLOAD_TYPE;
    set_next_payload_type(start[0]);
    cvalue = ( start[1] >> 6 ) & 0x3;
    data_length_value = (int)(( start[1] & 0x3F ) << 8 ) | (int)( start[2] );
    if( lengthLimit < 3 + data_length_value )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given dataPtr is too short to form a PKE Payload" );
        return;
    }
    data_ptr = new byte_t[ data_length_value ];
    memcpy( data_ptr, &start[3], data_length_value );
    end_ptr = start_ptr + 3 + data_length_value;

    assert( end_ptr - start_ptr == length() );
}

Mikey_Payload_PKE::~Mikey_Payload_PKE()
{
    if( data_ptr )
    {
        delete [] data_ptr;
    }
}

int Mikey_Payload_PKE::length()
{
    return 3 + data_length_value;
}

void Mikey_Payload_PKE::write_data( byte_t * start, int expectedLength )
{
    assert( expectedLength == length() );
    memset( start, 0, expectedLength );
    start[0] = (byte_t)next_payload_type();
    start[1] = (byte_t)(( cvalue & 0x3 ) << 6 | ( ( data_length_value >> 8 ) & 0x3F ));
    start[2] = (byte_t)( data_length_value & 0xFF );
    memcpy( &start[3], data_ptr, data_length_value );
}

std::string Mikey_Payload_PKE::debug_dump()
{
    return "Mikey_Payload_PKE: c=<" + itoa( cvalue ) +
            "> dataLengthValue=<" + itoa( data_length_value )+
            "> dataPtr=<" + bin_to_hex( data_ptr, data_length_value );
}

int Mikey_Payload_PKE::c()
{
    return cvalue;
}

int Mikey_Payload_PKE::data_length() const
{
    return data_length_value;
}
const byte_t * Mikey_Payload_PKE::data() const
{
    return data_ptr;
}
