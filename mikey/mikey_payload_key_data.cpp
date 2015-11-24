#include "mikey_payload_key_data.h"
#include "mikey_exception.h"
#include "string_utils.h"

#include<assert.h>
#include<string.h>
Mikey_Payload_Key_Data::Mikey_Payload_Key_Data( int type, byte_t * keyData, int keyDataLength, SRef<Key_Validity *> kv )
{
    this->payload_type_value = MIKEYPAYLOAD_KEYDATA_PAYLOAD_TYPE;
    if((type == KEYDATA_TYPE_TGK_SALT) || (type == KEYDATA_TYPE_TEK_SALT))
        throw Mikey_Exception( "This type of KeyData Payload requires a salt" );

    this->type_value = type;
    this->key_data_ptr = new byte_t[keyDataLength];
    this->key_data_length_value = keyDataLength;
    memcpy( this->key_data_ptr, keyData, keyDataLength );
    this->kv_ptr = kv;
    this->salt_data_length_value = 0;
    this->salt_data_ptr = NULL;
}

Mikey_Payload_Key_Data::Mikey_Payload_Key_Data( int type, byte_t * keyData, int keyDataLength, byte_t * saltData, int saltDataLength, SRef<Key_Validity *> kv )
{
    this->payload_type_value = MIKEYPAYLOAD_KEYDATA_PAYLOAD_TYPE;
    this->type_value = type;
    this->key_data_ptr = new byte_t[keyDataLength];
    this->key_data_length_value = keyDataLength;
    memcpy( this->key_data_ptr, keyData, keyDataLength );
    this->salt_data_ptr = new byte_t[saltDataLength];
    this->salt_data_length_value = saltDataLength;
    memcpy( this->salt_data_ptr, saltData, saltDataLength );
    this->kv_ptr = kv;
}

Mikey_Payload_Key_Data::Mikey_Payload_Key_Data( byte_t * start, int lengthLimit )
    : Mikey_Payload(start)
{
    int lengthWoKvPtr;
    this->payload_type_value = MIKEYPAYLOAD_KEYDATA_PAYLOAD_TYPE;
    if( lengthLimit < 4 )
        throw Mikey_Exception_Message_Length_Exception(
                "Given data is too short to form a KeyData Payload" );

    set_next_payload_type( start[0] );
    type_value = ( start[1] >> 4 ) & 0x0F;
    int kvPtrType = ( start[1]      ) & 0x0F;
    key_data_length_value = ( (int)start[2] ) << 8 | ( (int)start[3] );

    if( (type_value == KEYDATA_TYPE_TGK_SALT) || (type_value == KEYDATA_TYPE_TEK_SALT))
    {

        if( lengthLimit < 6 + key_data_length_value )
            throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a KeyData Payload" );
        key_data_ptr = new byte_t[ key_data_length_value ];
        memcpy( key_data_ptr, &start[4], key_data_length_value );
        salt_data_length_value =
                ( (int)start[2+key_data_length_value] ) << 8 |
                                                        ( (int)start[3+key_data_length_value] );
        if( lengthLimit < 6 + key_data_length_value + salt_data_length_value )
            throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a KeyData Payload" );
        salt_data_ptr = new byte_t[ salt_data_length_value ];
        memcpy( salt_data_ptr, &start[4+key_data_length_value],
                salt_data_length_value );
        lengthWoKvPtr = key_data_length_value + salt_data_length_value + 6;
    }
    else
    {
        if( lengthLimit < 4 + key_data_length_value )
            throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a KeyData Payload" );
        key_data_ptr = new byte_t[ key_data_length_value ];
        memcpy( key_data_ptr, &start[4], key_data_length_value );
        salt_data_length_value = 0;
        salt_data_ptr = NULL;
        lengthWoKvPtr = key_data_length_value + 4;
    }

    switch( kvPtrType )
    {
    case KEYVALIDITY_NULL:
        kv_ptr = new Key_Validity_Null();
        break;
    case KEYVALIDITY_SPI:
        kv_ptr = new Key_Validity_SPI( &start[lengthWoKvPtr], lengthLimit - lengthWoKvPtr );
        break;
    case KEYVALIDITY_INTERVAL:
        kv_ptr = new Key_Validity_Interval( &start[lengthWoKvPtr], lengthLimit - lengthWoKvPtr );
        break;
    default:
        throw Mikey_Exception_Message_Content( "Unknown key validity"
                    "type");
        break;
    }

    end_ptr = start_ptr + length();
}

Mikey_Payload_Key_Data::~Mikey_Payload_Key_Data()
{
    if( key_data_ptr != NULL )
        delete [] key_data_ptr;
    if( salt_data_ptr != NULL )
        delete [] salt_data_ptr;
}

void Mikey_Payload_Key_Data::write_data( byte_t * start, int expectedLength )
{
    assert( expectedLength == length() );
    start[0] = next_payload_type();
    start[1] = ( ( type_value & 0x0F  ) << 4 ) | ( kv_ptr->type() & 0x0F );
    start[2] = ( key_data_length_value >> 8 ) & 0xFF;
    start[3] = ( key_data_length_value      ) & 0xFF;

    memcpy( &start[4], key_data_ptr, key_data_length_value );

    if( ( type_value == KEYDATA_TYPE_TGK_SALT ) || ( type_value == KEYDATA_TYPE_TEK_SALT ) )
    {
        start[4 + key_data_length_value] = ( salt_data_length_value >> 8 ) & 0xFF;
        start[5 + key_data_length_value] = ( salt_data_length_value      ) & 0xFF;
        memcpy( &start[ 6 + key_data_length_value ], salt_data_ptr, salt_data_length_value);
        kv_ptr->write_data( &start[ 6 + key_data_length_value + salt_data_length_value], kv_ptr->length() );
    }
    else
    {
        kv_ptr->write_data( &start[ 4 + key_data_length_value ], kv_ptr->length() );
    }
}

int Mikey_Payload_Key_Data::length()
{
    return key_data_length_value + salt_data_length_value + kv_ptr->length() +
            (((   type_value == KEYDATA_TYPE_TGK_SALT)
              || (type_value == KEYDATA_TYPE_TEK_SALT))? 6:4 );
}

std::string Mikey_Payload_Key_Data::debug_dump()
{
    return "Mikey_Payload_Key_Data:"
            " nextPayloadType=<" + itoa( next_payload_type() ) +
            "> type=<" + itoa( type_value ) +
            "> keyDataPtr=<" + bin_to_hex( key_data_ptr, key_data_length_value ) +
            "> saltDataPtr=<" +
            bin_to_hex( salt_data_ptr, salt_data_length_value ) +
            "> kvPtr_type=<" + itoa( kv_ptr->type() ) +
            "> kvPtr_data=<" + kv_ptr->debug_dump() + ">";
}

int Mikey_Payload_Key_Data::type()
{
    return type_value;
}

SRef<Key_Validity *> Mikey_Payload_Key_Data::kv()
{
    return kv_ptr;
}

byte_t * Mikey_Payload_Key_Data::key_data()
{
    return key_data_ptr;
}

int Mikey_Payload_Key_Data::key_data_length()
{
    return key_data_length_value;
}

byte_t * Mikey_Payload_Key_Data::salt_data()
{
    return salt_data_ptr;
}

int Mikey_Payload_Key_Data::salt_data_length()
{
    return salt_data_length_value;
}
