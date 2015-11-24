#include "mikey_payload_sp.h"
#include "mikey_exception.h"
#include "string_utils.h"

#include <stdlib.h>
#include <list>
using namespace std;

Mikey_Policy_Param::Mikey_Policy_Param( uint8_t type, uint8_t length, byte_t * value )
    : type(type), length(length)
{
    this->type = type;
    this->length = length;
    this->value = (byte_t*) calloc (length,sizeof(byte_t));
    for(int i=0; i< length; i++)
        this->value[i] = value[i];
}

Mikey_Policy_Param::~Mikey_Policy_Param()
{
    free(value);
}

Mikey_Payload_SP::Mikey_Payload_SP(uint8_t policy_no, uint8_t prot_type)
{
    this->payload_type_value = MIKEYPAYLOAD_SP_PAYLOAD_TYPE;
    this->policy_param_length = 0;
    this->policy_no = policy_no;
    this->prot_type = prot_type;
}

Mikey_Payload_SP::Mikey_Payload_SP(byte_t *start, int lengthLimit)
    : Mikey_Payload(start)
{
    this->payload_type_value = MIKEYPAYLOAD_SP_PAYLOAD_TYPE;
    this->policy_param_length = 0;
    this->next_payload_type_value = start[0];
    this->policy_no = start[1];
    this->prot_type = start[2];
    int i = 5;
    uint16_t j = ((uint16_t)start[3] << 8 | (uint16_t)start[4]) + 5;
    //byte_t *value;
    end_ptr = start_ptr + j;
    //while(i < lengthLimit) {
    while(i < j )
    {
        this->add_mikey_policy_param(start[i], start[i+1], &start[i+2]);
        i = i + 2 + start[i+1];
    }
    assert (end_ptr - start_ptr == length() );
}

Mikey_Payload_SP::~Mikey_Payload_SP()
{
    list<Mikey_Policy_Param *>::iterator i;
    for( i = param.begin(); i != param.end() ; i++ )
        delete *i;
    param.clear();
}


void Mikey_Payload_SP::add_mikey_policy_param( uint8_t type, uint8_t length, byte_t * value)
{
    if(this->get_parameter_type(type) != NULL)
        this->delete_mikey_policy_param(type);

    param.push_back (new Mikey_Policy_Param(type, length, value));
    this->policy_param_length = this->policy_param_length + length + 2;
}

Mikey_Policy_Param * Mikey_Payload_SP::get_parameter_type(uint8_t type)
{
    list<Mikey_Policy_Param *>::iterator i;
    for( i = param.begin(); i != param.end()  ; i++ )
    {
        if( (*i)->type == type )
            return *i;
    }
    return NULL;
}

void Mikey_Payload_SP::write_data(byte_t *start, int expectedLength)
{
    assert( expectedLength == this->length() );
    start[0] = this->next_payload_type_value;
    start[1] = this->policy_no;
    start[2] = this->prot_type;
    start[3] = (byte_t) ((this->policy_param_length & 0xFF00) >> 8);
    start[4] = (byte_t) (this->policy_param_length & 0xFF);
    //Add policy params
    list<Mikey_Policy_Param *>::iterator i = param.begin();
    int j=5,k;
    while (i != param.end() && j < expectedLength)
    {
        start[j++] = (*i)->type;
        start[j++] = (*i)->length;
        for(k=0; k < ((*i)->length); k++)
            start[j++] = ((*i)->value)[k];
        i++;
    }
}

int Mikey_Payload_SP::length()
{
    return 5 + this->policy_param_length;
}

int Mikey_Payload_SP::no_of_policy_param()
{
    return (int)param.size();
}

std::string Mikey_Payload_SP::debug_dump()
{
    string ret = "Mikey_Payload_SP: next_payload<" + itoa( next_payload_type_value ) + "> ";

    ret += string("policyNo: <") + itoa( policy_no ) + "> ";
    ret += string("protType: <") + itoa( prot_type ) + ">\n";

    list<Mikey_Policy_Param *>::iterator i = param.begin();
    for( ; i != param.end(); i++ )
    {
        ret += string("type: <") + itoa( (*i)->type ) + "> ";
        ret += string("value: <")
                + bin_to_hex( (*i)->value, (*i)->length ) + ">\n";
    }

    return ret;
}
void Mikey_Payload_SP::delete_mikey_policy_param(uint8_t type)
{
    std::list<Mikey_Policy_Param *>::iterator i;
    for( i = param.begin(); i != param.end()  ; i++ )
        if( (*i)->type == type )
        {
            this->policy_param_length = this->policy_param_length - (*i)->length - 2;
            delete *i;
            i = param.erase(i);
        }
}
