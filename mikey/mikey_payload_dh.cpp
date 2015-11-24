#include "mikey_payload_dh.h"

#include "mikey_exception.h"
#include "string_utils.h"

#include<assert.h>
#include<string.h>


Mikey_Payload_DH::Mikey_Payload_DH( int dhGroup, byte_t * dhKey, SRef<Key_Validity *> kv )
{
    this->payload_type_value = MIKEYPAYLOAD_DH_PAYLOAD_TYPE;
    this->dh_group = dhGroup;

    switch( dhGroup )
    {
    case MIKEYPAYLOAD_DH_GROUP5:
        dh_key_length_value = 192;
        break;
    case MIKEYPAYLOAD_DH_GROUP1:
        dh_key_length_value = 96;
        break;
    case MIKEYPAYLOAD_DH_GROUP2:
        dh_key_length_value = 128;
        break;
    default:
        throw Mikey_Exception_Message_Content("Unknown DH group");
        break;
    }

    this->dh_key_ptr = new byte_t[ dh_key_length_value ];
    memcpy( this->dh_key_ptr, dhKey, dh_key_length_value );

    this->kv_ptr = kv;
}

Mikey_Payload_DH::Mikey_Payload_DH( byte_t * start, int lengthLimit )
    : Mikey_Payload( start )
{
    this->payload_type_value = MIKEYPAYLOAD_DH_PAYLOAD_TYPE;
    if( lengthLimit < 3 )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a DH Payload" );
        return;
    }
    set_next_payload_type( start[0] );
    dh_group = start[1];
    switch( dh_group )
    {
    case MIKEYPAYLOAD_DH_GROUP5:
        dh_key_length_value = 192;
        break;
    case MIKEYPAYLOAD_DH_GROUP1:
        dh_key_length_value = 96;
        break;
    case MIKEYPAYLOAD_DH_GROUP2:
        dh_key_length_value = 128;
        break;
    default:
        throw Mikey_Exception_Message_Content( "Unknown DH group" );
        break;
    }

    if( lengthLimit < 3 + dh_key_length_value )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a DH Payload" );
        return;
    }

    dh_key_ptr = new byte_t[ dh_key_length_value ];
    memcpy( dh_key_ptr, &start[2], dh_key_length_value );

    int kvType = start[2+dh_key_length_value] & 0x0F;
    switch( kvType )
    {
    case KEYVALIDITY_NULL:
        kv_ptr = new Key_Validity_Null();
        break;
    case KEYVALIDITY_SPI:
        kv_ptr = new Key_Validity_SPI(
                    &start[ 3 + dh_key_length_value ],
                lengthLimit - 3 - dh_key_length_value );
        break;
    case KEYVALIDITY_INTERVAL:
        kv_ptr = new Key_Validity_Interval(
                    &start[ 3 + dh_key_length_value ],
                lengthLimit - 3 - dh_key_length_value );
        break;
    default:
        throw Mikey_Exception_Message_Content(
                    "Unknown DH key validity "
                    "type");
        break;
    }

    if( lengthLimit < 3 + dh_key_length_value + kv_ptr->length() )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a DH Payload" );
        return;
    }
    end_ptr = start_ptr + 3 + dh_key_length_value + kv_ptr->length();

    assert( end_ptr - start_ptr == length() );
}

Mikey_Payload_DH::~Mikey_Payload_DH()
{
    if( dh_key_ptr )
        delete [] dh_key_ptr;
}

void Mikey_Payload_DH::write_data( byte_t * start, int expectedLength )
{
    assert( expectedLength == length() );
    memset( start, 0, expectedLength );
    start[0] = next_payload_type();
    start[1] = dh_group;
    memcpy( &start[2], dh_key_ptr, dh_key_length_value );
    start[ 2 + dh_key_length_value ] = 0x0F & kv_ptr->type();
    kv_ptr->write_data( &start[ 3 + dh_key_length_value ], kv_ptr->length() );
}

int Mikey_Payload_DH::length()
{
    return 3 + kv_ptr->length() + dh_key_length_value;
}

std::string Mikey_Payload_DH::debug_dump()
{
    return "Mikey_Payload_DH: "
            "nextPayloadType=<" + itoa( next_payload_type_value ) +
            "> dhGroup=<" + itoa( dh_group ) +
            "> dhKeyPtr=<" + bin_to_hex( dh_key_ptr, dh_key_length_value ) +
            "> kvType=<" + itoa( kv_ptr->type() ) +
            ">" + kv_ptr->debug_dump();
}

int Mikey_Payload_DH::group()
{
    return dh_group;
}

byte_t * Mikey_Payload_DH::dh_key()
{
    return dh_key_ptr;
}

int Mikey_Payload_DH::dh_key_length()
{
    return dh_key_length_value;
}

SRef<Key_Validity *> Mikey_Payload_DH::kv()
{
    return kv_ptr;
}
