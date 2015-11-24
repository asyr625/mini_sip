#include "mikey_payload_kemac.h"
#include "mikey_exception.h"
#include "mikey_message.h"

#include<assert.h>
#include<string.h>

#include "aes.h"
#include "string_utils.h"

Mikey_Payload_KEMAC::Mikey_Payload_KEMAC( int encr_alg, int encr_dataLength, byte_t * encr_data, int mac_alg, byte_t * mac_data )
{
    this->payload_type_value = MIKEYPAYLOAD_KEMAC_PAYLOAD_TYPE;
    this->encr_alg_value = encr_alg;
    this->encr_data_length_value = encr_dataLength;
    this->encr_data_ptr = new byte_t[ encr_dataLength ];
    memcpy( this->encr_data_ptr, encr_data, encr_dataLength );
    this->mac_alg_value = mac_alg;
    switch( mac_alg )
    {
    case MIKEY_PAYLOAD_KEMAC_MAC_HMAC_SHA1_160:
        this->mac_data_ptr = new byte_t[ 20 ];
        memcpy( this->mac_data_ptr, mac_data, 20 );
        break;
    case MIKEY_PAYLOAD_KEMAC_MAC_NULL:
        this->mac_data_ptr = NULL;
        break;
    default:
        throw Mikey_Exception_Message_Content(
                    "Unknown MAC algorithm in KEYMAC Payload (1)" );
    }
}

Mikey_Payload_KEMAC::Mikey_Payload_KEMAC( byte_t *start, int lengthLimit )
    : Mikey_Payload(start)
{
    if(  lengthLimit < 5 )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a KEMAC Payload" );
        return;
    }
    this->payload_type_value = MIKEYPAYLOAD_KEMAC_PAYLOAD_TYPE;
    set_next_payload_type(start[0]);
    encr_alg_value = start[1];
    encr_data_length_value = ( (int)start[2] << 8) | (int)start[3];
    if( lengthLimit < 5 + encr_data_length_value )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a KEMAC Payload" );
        return;
    }
    mac_alg_value = (int)start[4 + encr_data_length_value];
    switch( mac_alg_value )
    {
    case MIKEY_PAYLOAD_KEMAC_MAC_HMAC_SHA1_160:
        if( lengthLimit < 25 + encr_data_length_value )
        {
            throw Mikey_Exception_Message_Length_Exception(
                        "Given data is too short to form"
                        "a KEMAC Payload" );
            return;
        }
        this->mac_data_ptr = new byte_t[ 20 ];
        memcpy( this->mac_data_ptr, & start[5 + encr_data_length_value], 20 );
        end_ptr = start_ptr + 25 + encr_data_length_value;
        break;
    case MIKEY_PAYLOAD_KEMAC_MAC_NULL:
        this->mac_data_ptr = NULL;
        end_ptr = start_ptr + 5 + encr_data_length_value;
        break;
    default:
        throw Mikey_Exception_Message_Content(
                    "Unknown MAC algorithm in KEYMAC Payload (2)" );
        return;
    }

    encr_data_ptr = new byte_t[ encr_data_length_value ];
    memcpy( encr_data_ptr, &start[4], encr_data_length_value );

    assert( end_ptr - start_ptr == length() );
}

Mikey_Payload_KEMAC::~Mikey_Payload_KEMAC()
{
    if( encr_data_ptr != NULL )
        delete [] encr_data_ptr;
    if( mac_data_ptr != NULL )
        delete [] mac_data_ptr;
}

int Mikey_Payload_KEMAC::length()
{
    return  5 + encr_data_length_value +
            ( (mac_alg_value == MIKEY_PAYLOAD_KEMAC_MAC_HMAC_SHA1_160)?20:0 );
}

void Mikey_Payload_KEMAC::write_data( byte_t *start, int expectedLength )
{
    assert( expectedLength == length() );
    start[0] = (byte_t)next_payload_type();
    start[1] = (byte_t)( encr_alg_value & 0xFF );
    start[2] = (byte_t)( (encr_data_length_value >> 8) & 0xFF );
    start[3] = (byte_t)( (encr_data_length_value) & 0xFF );

    memcpy( &start[4], encr_data_ptr, encr_data_length_value );

    start[4 + encr_data_length_value] = (byte_t)( mac_alg_value&0xFF );
    if( mac_alg_value == MIKEY_PAYLOAD_KEMAC_MAC_HMAC_SHA1_160 )
        memcpy( &start[5 + encr_data_length_value], mac_data_ptr, 20 );
}

std::string Mikey_Payload_KEMAC::debug_dump()
{
    return "Mikey_Payload_KEMAC: encrAlgValue=<" + itoa( encr_alg_value ) +
            "> encrDataLengthValue=<" + itoa( encr_data_length_value ) +
            "> encrDataPtr=<" +
            bin_to_hex( encr_data_ptr, encr_data_length_value ) +
            "> macAlgValue=<" + itoa( mac_alg_value ) +
            "> macDataPtr=<" + bin_to_hex( mac_data_ptr,
                                           ((mac_alg_value == MIKEY_PAYLOAD_KEMAC_MAC_HMAC_SHA1_160)?20:0)
                                           ) + ">";
}

int Mikey_Payload_KEMAC::encr_alg()
{
    return encr_alg_value;
}

int Mikey_Payload_KEMAC::encr_data_length()
{
    return encr_data_length_value;
}

byte_t * Mikey_Payload_KEMAC::encr_data()
{
    return encr_data_ptr;
}

Mikey_Payloads* Mikey_Payload_KEMAC::decode_payloads( int firstPayloadType, byte_t * encrKey, int encrKeyLength, byte_t * iv )
{
    byte_t * decrData = new byte_t[ encr_data_length_value ];
    AES * aes;

    switch( encr_alg_value)
    {
    case MIKEY_PAYLOAD_KEMAC_ENCR_AES_CM_128:
        aes = new AES( encrKey, encrKeyLength );
        aes->ctr_encrypt( encr_data_ptr, encr_data_length_value, decrData, iv );
        delete aes;
        break;
    case MIKEY_PAYLOAD_KEMAC_ENCR_NULL:
        memcpy( decrData, encr_data_ptr, encr_data_length_value );
        break;
    case MIKEY_PAYLOAD_KEMAC_ENCR_AES_KW_128:
        //TODO
    default:
        delete [] decrData;
        throw Mikey_Exception(
                    "Unknown encryption algorithm" );
        break;
    }

    Mikey_Payloads *output =
            new Mikey_Payloads( firstPayloadType, decrData, encr_data_length_value );
    // decrData is owned and deleted by MikeyPayloads
    return output;
}

int Mikey_Payload_KEMAC::mac_alg()
{
    return mac_alg_value;
}

byte_t * Mikey_Payload_KEMAC::mac_data()
{
    return mac_data_ptr;
}

void Mikey_Payload_KEMAC::set_mac( byte_t * data )
{
    if( mac_data_ptr != NULL )
        delete [] mac_data_ptr;

    switch( mac_alg_value )
    {
    case MIKEY_PAYLOAD_KEMAC_MAC_HMAC_SHA1_160:
        mac_data_ptr = new byte_t[ 20 ];
        memcpy( mac_data_ptr, data, 20 );
        break;
    case MIKEY_PAYLOAD_KEMAC_MAC_NULL:
        mac_data_ptr = NULL;
        break;
    default:
        throw Mikey_Exception( "Unknown MAC algorithm (PayloadKEMAC::setMac)" );
    }
}
