#include "mikey_payload_chash.h"


Mikey_Payload_CHASH::Mikey_Payload_CHASH(byte_t *start, int lengthLimit)
    : Mikey_Payload(start)
{
}

void Mikey_Payload_CHASH::write_data(byte_t *start, int expectedLength)
{
}

int Mikey_Payload_CHASH::length()
{
    return 0;
}
