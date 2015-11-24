#include "mikey_payload_rand.h"
#include "mikey_exception.h"
#include "string_utils.h"
#include "rand.h"

#include<assert.h>
#include<stdlib.h>
#include<time.h>
#include<sys/types.h>
#include<string.h>

#ifndef _MSC_VER
#include<unistd.h>
#endif

using namespace std;

Mikey_Payload_RAND::Mikey_Payload_RAND()
    : Mikey_Payload()
{
    this->payload_type_value = MIKEYPAYLOAD_RAND_PAYLOAD_TYPE;
    rand_length_value = 16;

    rand_data_ptr = new byte_t[ rand_length_value ];
    Rand::randomize( rand_data_ptr, rand_length_value );
}

Mikey_Payload_RAND::Mikey_Payload_RAND( int randlen, byte_t * rand_data )
{
    this->payload_type_value = MIKEYPAYLOAD_RAND_PAYLOAD_TYPE;
    this->rand_length_value = randlen;
    this->rand_data_ptr = new byte_t[ randlen ];
    memcpy( this->rand_data_ptr, rand_data, randlen );
}

Mikey_Payload_RAND::Mikey_Payload_RAND( byte_t * start, int lengthLimit )
    : Mikey_Payload( start )
{

    this->payload_type_value = MIKEYPAYLOAD_RAND_PAYLOAD_TYPE;
    if( lengthLimit < 2 )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a RAND Payload" );
        return;
    }

    set_next_payload_type( start[0] );
    rand_length_value = start[1];
    if( lengthLimit < 2 + rand_length_value )
    {
        throw Mikey_Exception_Message_Length_Exception( "Given data is too short to form a RAND Payload" );
        return;
    }
    rand_data_ptr = new byte_t[ rand_length_value ];
    memcpy( rand_data_ptr, &start[2],rand_length_value );
    end_ptr = start_ptr + 2 + rand_length_value;

    assert( end_ptr - start_ptr == length() );
}

Mikey_Payload_RAND::Mikey_Payload_RAND(SRef<Sip_Sim *> sim)
    : Mikey_Payload()
{
    this->payload_type_value = MIKEYPAYLOAD_RAND_PAYLOAD_TYPE;
    rand_length_value = 16;
    rand_data_ptr = new byte_t[ rand_length_value ];
    Rand::randomize(rand_data_ptr, rand_length_value, sim);
}

Mikey_Payload_RAND::~Mikey_Payload_RAND()
{
    if( rand_data_ptr )
    {
        delete [] rand_data_ptr;
    }
    rand_data_ptr=NULL;
}

int Mikey_Payload_RAND::length()
{
    return 2 + rand_length_value;
}

void Mikey_Payload_RAND::write_data( byte_t * start, int expectedLength )
{
    assert( expectedLength == length() );
    start[0] = next_payload_type();
    start[1] = rand_length_value;
    memcpy( &start[2], rand_data_ptr, rand_length_value );
}

std::string Mikey_Payload_RAND::debug_dump()
{
    return "Mikey_Payload_RAND: nextPayloadType=<" +
            itoa( next_payload_type() ) +
            "> randLengthValue=<" + itoa( rand_length_value ) +
            "> randDataPtr=<" + bin_to_hex( rand_data_ptr, rand_length_value ) +
            ">";
}

int Mikey_Payload_RAND::rand_length()
{
    return rand_length_value;
}

byte_t * Mikey_Payload_RAND::rand_data()
{
    return rand_data_ptr;
}
