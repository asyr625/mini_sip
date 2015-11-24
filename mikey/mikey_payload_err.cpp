#include "mikey_payload_err.h"
#include "mikey_exception.h"
#include "string_utils.h"

Mikey_Payload_ERR::Mikey_Payload_ERR( int errType )
{
    this->payload_type_value = MIKEYPAYLOAD_ERR_PAYLOAD_TYPE;
    this->err_type_value = errType;
}

Mikey_Payload_ERR::Mikey_Payload_ERR( byte_t * start, int lengthLimit )
    : Mikey_Payload(start)
{
    this->payload_type_value = MIKEYPAYLOAD_ERR_PAYLOAD_TYPE;
    if( lengthLimit < 4 )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a ERR Payload" );
    }

    set_next_payload_type( start[0] );
    this->err_type_value = start[1];
    end_ptr = start_ptr + 4;

    assert( end_ptr - start_ptr == length() );
}

Mikey_Payload_ERR::~Mikey_Payload_ERR()
{
}

void Mikey_Payload_ERR::write_data( byte_t * start, int expectedLength )
{
    assert( expectedLength == length() );
    start[0] = next_payload_type();
    start[1] = err_type_value & 0xFF;
    memset( &start[2], 0, 2 );
}

int Mikey_Payload_ERR::length()
{
    return 4;
}

std::string Mikey_Payload_ERR::debug_dump()
{
    return "Mikey_Payload_ERR: nextPayloadType=<" +
            itoa( next_payload_type_value ) + "> err_type=<" +
            itoa( err_type_value ) + ">";
}

int Mikey_Payload_ERR::error_type()
{
    return err_type_value;
}
