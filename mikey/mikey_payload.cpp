#include "mikey_payload.h"
#include "mikey_exception.h"

const int Mikey_Payload::Last_Payload = 0;

Mikey_Payload::Mikey_Payload()
    : raw_packet_valid(false),
      start_ptr(NULL),
      end_ptr(NULL),
      next_payload_type_value(0)
{
}

Mikey_Payload::Mikey_Payload( byte_t *start )
    : raw_packet_valid(true),
      start_ptr(start),
      end_ptr(NULL),
      next_payload_type_value(0)
{
}

Mikey_Payload::~Mikey_Payload()
{
}

int Mikey_Payload::next_payload_type()
{
    if( next_payload_type_value == -1 )
        throw Mikey_Exception_Uninitialized( "Next payload attribute in payload"
                                             "was not initialized. (this is probably a BUG!)" );
    return next_payload_type_value;
}

void Mikey_Payload::set_next_payload_type(int t)
{
    next_payload_type_value = t;
}

int Mikey_Payload::payload_type()
{
    return payload_type_value;
}

byte_t * Mikey_Payload::end()
{
    return end_ptr;
}
