#include "mikey_payload_general_extensions.h"
#include <stdlib.h>
#include <assert.h>
Mikey_Payload_General_Extensions::Mikey_Payload_General_Extensions(byte_t *start, int lengthLimit)
    : Mikey_Payload(start)
{
    this->payload_type_value = MIKEYPAYLOAD_GENERALEXTENSIONS_PAYLOAD_TYPE;
    this->next_payload_type_value = start[0];
    this->type = start[1];
    this->leng = (uint16_t)start[2] << 8 | (uint16_t)start[3];
    this->data  = (byte_t*) calloc (lengthLimit - 4 ,sizeof(byte_t));
    for(int i=4; i< lengthLimit; i++)
        this->data[i] = start[i];
    end_ptr = start_ptr + this->leng + 4;
    assert (end_ptr - start_ptr == length() );
}

Mikey_Payload_General_Extensions::Mikey_Payload_General_Extensions(uint8_t type, uint16_t length, byte_t * data)
{
    this->payload_type_value = MIKEYPAYLOAD_GENERALEXTENSIONS_PAYLOAD_TYPE;
    this->type = type;
    this->leng = length;
    this->data  = (byte_t*) calloc (length ,sizeof(byte_t));
    for(int i=4; i< length; i++)
        this->data[i] = data[i];
}

Mikey_Payload_General_Extensions::~Mikey_Payload_General_Extensions()
{
    free(data);
}

void Mikey_Payload_General_Extensions::write_data(byte_t *start, int expectedLength)
{
    assert( expectedLength == this->length() );
    start[0] = this->next_payload_type_value;
    start[1] = this->type;
    start[2] = (byte_t) ((this->leng & 0xFF00) >> 8);
    start[3] = (byte_t) (this->leng & 0xFF);
    for(int i= 4; i< expectedLength; i++)
        start[i] = data[i-4];
}

int Mikey_Payload_General_Extensions::length()
{
    return this->leng + 4;
}
