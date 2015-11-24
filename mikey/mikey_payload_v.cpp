#include "mikey_payload_v.h"
#include "mikey_exception.h"
#include "string_utils.h"

Mikey_Payload_V::Mikey_Payload_V( int mac_alg, byte_t * verData )
{
    this->payload_type_value = MIKEYPAYLOAD_V_PAYLOAD_TYPE;
    this->mac_alg_value = mac_alg;
    switch( mac_alg_value )
    {
    case MIKEY_PAYLOAD_V_MAC_HMAC_SHA1_160:
        this->ver_data_ptr = new byte_t[20];
        memcpy( this->ver_data_ptr, verData, 20 );
        break;
    case MIKEY_PAYLOAD_V_MAC_NULL:
        this->ver_data_ptr = NULL;
        break;
    default:
        throw Mikey_Exception_Message_Content(
                    "Unknown MAC algorithm in V payload (1)" );
        return;
    }
}

Mikey_Payload_V::Mikey_Payload_V( byte_t * start, int lengthLimit )
    : Mikey_Payload( start )
{
    this->payload_type_value = MIKEYPAYLOAD_V_PAYLOAD_TYPE;
    if( lengthLimit < 2 )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a V Payload" );
        return;
    }

    set_next_payload_type( start[0] );
    this->mac_alg_value = (int)start[1];
    switch( mac_alg_value )
    {
    case MIKEY_PAYLOAD_V_MAC_HMAC_SHA1_160:
        if( lengthLimit < 22 )
        {
            throw Mikey_Exception_Message_Length_Exception( "Given data is too short to"
                                                            "form a V Payload" );
            return;
        }
        ver_data_ptr = new byte_t[ 20 ];
        memcpy( ver_data_ptr, &start[2], 20 );
        end_ptr = start_ptr + 22;
        break;
    case MIKEY_PAYLOAD_V_MAC_NULL:
        ver_data_ptr = NULL;
        end_ptr = start_ptr + 2;
        break;
    default:
        throw Mikey_Exception_Message_Content( "Unknown MAC algorithm in V payload (2)" );
        return;
    }
}

Mikey_Payload_V::~Mikey_Payload_V()
{
    if( ver_data_ptr )
    {
        delete[] ver_data_ptr;
        ver_data_ptr = NULL;
    }
}

void Mikey_Payload_V::write_data( byte_t * start, int expectedLength )
{
    assert( expectedLength == length() );
    start[0] = (byte_t)next_payload_type();
    start[1] = (byte_t)( mac_alg_value & 0xFF );
    if( mac_alg_value == MIKEY_PAYLOAD_V_MAC_HMAC_SHA1_160 )
        memcpy( &start[2], ver_data_ptr, 20 );
}

int Mikey_Payload_V::length()
{
    return 2 + (( mac_alg_value == MIKEY_PAYLOAD_V_MAC_HMAC_SHA1_160 )?20:0) ;
}

int Mikey_Payload_V::mac_alg()
{
    return mac_alg_value;
}

byte_t * Mikey_Payload_V::ver_data()
{
    return ver_data_ptr;
}

void Mikey_Payload_V::set_mac( byte_t * data )
{
    if( ver_data_ptr )
        delete [] ver_data_ptr;

    switch( mac_alg_value )
    {
    case MIKEY_PAYLOAD_V_MAC_HMAC_SHA1_160:
        ver_data_ptr = new byte_t[ 20 ];
        memcpy( ver_data_ptr, data, 20 );
        break;
    case MIKEY_PAYLOAD_V_MAC_NULL:
        ver_data_ptr = NULL;
        break;
    default:
        throw Mikey_Exception("Unknown MAC algorithm (Payload V::setMac)" );
    }
}
