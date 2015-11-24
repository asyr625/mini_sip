#include "mikey_payload_id.h"
#include "mikey_exception.h"
#include "string_utils.h"

Mikey_Payload_ID::Mikey_Payload_ID( int type, int length, byte_t * data )
{
    this->payload_type_value = MIKEYPAYLOAD_ID_PAYLOAD_TYPE;
    this->id_type_value = type;
    this->id_length_value = length;
    this->id_data_ptr = new byte_t[ length ];
    memcpy( this->id_data_ptr, data, length );
}

Mikey_Payload_ID::Mikey_Payload_ID( byte_t * start, int lengthLimit )
    : Mikey_Payload(start)
{
    if( lengthLimit < 4 )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a ID Payload" );
        return;
    }
    this->payload_type_value = MIKEYPAYLOAD_ID_PAYLOAD_TYPE;
    set_next_payload_type( start[0] );
    id_type_value = start[1];
    id_length_value = (int)( start[ 2 ] ) << 8 | start[ 3 ];
    if( lengthLimit < 4 + id_length_value )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a ID Payload" );
        return;
    }

    id_data_ptr = new byte_t[ id_length_value ];
    memcpy( id_data_ptr, &start[4], id_length_value );
    end_ptr = start_ptr + 4 + id_length_value;

    assert( end_ptr - start_ptr == length() );
}

Mikey_Payload_ID::~Mikey_Payload_ID()
{
    if( id_data_ptr )
        delete [] id_data_ptr;
    id_data_ptr = NULL;
}

void Mikey_Payload_ID::write_data(byte_t *start, int expectedLength)
{
    assert( expectedLength == length() );
    memset( start, 0, expectedLength );
    start[0] = next_payload_type();
    start[1] = id_type_value;
    start[2] = (byte_t) ((id_length_value & 0xFF00) >> 8);
    start[3] = (byte_t) (id_length_value & 0xFF);
    memcpy( &start[4], id_data_ptr, id_length_value );
}

int Mikey_Payload_ID::length()
{
    return 4 + id_length_value;
}

std::string Mikey_Payload_ID::debug_dump()
{
    return "Mikey_Payload_ID: nextPayloadType=<" + itoa( next_payload_type() ) +
            "> type=<" + itoa( id_type_value ) +
            "> length=<" + itoa( id_length_value ) +
            "> data=<" + bin_to_hex( id_data_ptr, id_length_value ) + ">";
}

int Mikey_Payload_ID::id_type() const
{
    return id_type_value;
}
int Mikey_Payload_ID::id_length() const
{
    return id_length_value;
}
const byte_t * Mikey_Payload_ID::id_data() const
{
    return id_data_ptr;
}
