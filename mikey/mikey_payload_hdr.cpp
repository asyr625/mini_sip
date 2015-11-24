#include "mikey_payload_hdr.h"
#include "string_utils.h"
#include "mikey_exception.h"

Mikey_Payload_HDR::Mikey_Payload_HDR( byte_t *start, int lengthLimit )
    : Mikey_Payload( start )
{
    this->payload_type_value = MIKEYPAYLOAD_HDR_PAYLOAD_TYPE;
    if( lengthLimit < 10 )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a HDR Payload" );
        return;
    }

    set_next_payload_type( start[2] );
    this->version = start[0];
    this->data_type_value = start[1];
    this->vvalue = ( start[3] >> 7 ) & 0x1;
    this->prf_func = start[3] & 0x7F;
    this->csb_id_value = (int)start[4] << 24 |
                                        (int)start[5] << 16 |
                                                         (int)start[6] <<  8 |
                                                                           (int)start[7];
    this->ncs_value = start[8];
    this->cs_id_map_type_value = start[9];
    if( cs_id_map_type_value == HDR_CS_ID_MAP_TYPE_SRTP_ID ||
            cs_id_map_type_value == HDR_CS_ID_MAP_TYPE_IPSEC4_ID)
    {
        if( lengthLimit < 10 + ncs_value * 9 )
        {
            throw Mikey_Exception_Message_Length_Exception(
                        "Given data is too short to form any HDR Payload" );
            return;
        }
        if( cs_id_map_type_value == HDR_CS_ID_MAP_TYPE_SRTP_ID )
        {
            this->cs_id_map_ptr =
                    new Mikey_Cs_Id_Map_Srtp( &start[10], 9 * ncs_value );
            this->end_ptr = start_ptr + 10 + 9 * this->ncs_value;
        }
        if( cs_id_map_type_value == HDR_CS_ID_MAP_TYPE_IPSEC4_ID )
        {
            this->cs_id_map_ptr =
                    new Mikey_Cs_Id_Map_IPSEC4( &start[10], 13 * ncs_value );
            this->end_ptr = start_ptr + 10 + 13 * this->ncs_value;
        }

    }
    else{
        throw Mikey_Exception_Message_Content(
                    "Unknown type of CS_ID_map" );
        this->cs_id_map_ptr = NULL;
    }
}

Mikey_Payload_HDR::Mikey_Payload_HDR( int data_type, int V, int PRF_func, uint32_t CSB_id, int n_cs, int map_type, SRef<Mikey_Cs_Id_Map *> map )
{
    this->payload_type_value = MIKEYPAYLOAD_HDR_PAYLOAD_TYPE;
    this->version = 1;
    this->data_type_value = data_type;
    this->vvalue = V;
    this->ncs_value = n_cs;
    this->prf_func = PRF_func;
    this->csb_id_value = CSB_id;
    if( map_type == HDR_CS_ID_MAP_TYPE_SRTP_ID || map_type == HDR_CS_ID_MAP_TYPE_IPSEC4_ID)
    {
        this->cs_id_map_type_value = map_type;
        this->cs_id_map_ptr = map;
    }
    else
    {
        throw Mikey_Exception_Message_Content( "Unknown CS ID map type" );
    }

    if( !cs_id_map_ptr )
    {
        throw Mikey_Exception_Message_Content( "Missing CS ID map" );
    }
}


Mikey_Payload_HDR::~Mikey_Payload_HDR()
{
}

int Mikey_Payload_HDR::length()
{
    return 10 + cs_id_map_ptr->length();
}

void Mikey_Payload_HDR::write_data( byte_t * start, int expectedLength )
{
    assert( expectedLength == length() );
    memset( start, 0, expectedLength );
    start[0] = (byte_t) version;
    start[1] = (byte_t) data_type_value;
    start[2] = next_payload_type();
    start[3] = ( (byte_t)vvalue & 0x1) << 7
                                          | ( (byte_t)prf_func & 0x7F );
    start[4] = (byte_t) ((csb_id_value & 0xFF000000) >> 24);
    start[5] = (byte_t) ((csb_id_value & 0xFF0000) >> 16);
    start[6] = (byte_t) ((csb_id_value & 0xFF00) >> 8);
    start[7] = (byte_t) (csb_id_value & 0xFF);
    start[8] = (byte_t) ncs_value;
    start[9] = (byte_t) cs_id_map_type_value;
    cs_id_map_ptr->write_data( &start[10], cs_id_map_ptr->length() );
}

std::string Mikey_Payload_HDR::debug_dump()
{
    std::string ret= "Mikey_Payload_HDR: version=<"+itoa(version)+"> datatype=";
    switch( data_type_value )
    {
    case  HDR_DATA_TYPE_PSK_INIT:
        ret=ret+"<Pre-shared>";
        break;
    case HDR_DATA_TYPE_PSK_RESP:
        ret=ret+"<PS ver msg>";
        break;
    case HDR_DATA_TYPE_PK_INIT:
        ret=ret+"<Public key>";
        break;
    case HDR_DATA_TYPE_PK_RESP:
        ret=ret+"<PK ver msg>";
        break;
    case  HDR_DATA_TYPE_DH_INIT:
        ret=ret+"<D-H init>";
        break;
    case HDR_DATA_TYPE_DH_RESP:
        ret=ret+"<D-H resp>";
        break;
    case HDR_DATA_TYPE_DHHMAC_INIT:
        ret=ret+"<DHMAC init>";
        break;
    case HDR_DATA_TYPE_DHHMAC_RESP:
        ret=ret+"<DHMAC resp>";
        break;
    case HDR_DATA_TYPE_RSA_R_INIT:
        ret=ret+"<RSA-R I_MSG>";
        break;
    case HDR_DATA_TYPE_RSA_R_RESP:
        ret=ret+"<RSA-R R_MSG>";
        break;
    case HDR_DATA_TYPE_ERROR:
        ret=ret+"<Error>";
        break;
    }

    ret += " next_payload=" + itoa( next_payload_type() );
    ret += " V=" + itoa( vvalue );
    ret += " PRF_func=";
    switch( prf_func )
    {
    case HDR_PRF_MIKEY_1:
        ret += "<MIKEY-1>";
        break;
    case  HDR_PRF_MIKEY_256:
        ret += "<MIKEY-256>";
        break;
    case HDR_PRF_MIKEY_384:
        ret += "<MIKEY-384>";
        break;
    case  HDR_PRF_MIKEY_512:
        ret += "<MIKEY-512>";
        break;
    }

    ret += " CSB_id=<" + itoa( csb_id_value ) + ">";
    ret += " #CS=<" + itoa( ncs_value );
    ret += " CS ID map type=";
    if ( cs_id_map_type_value == HDR_CS_ID_MAP_TYPE_SRTP_ID )
        ret += "<SRTP-ID>";
    if ( cs_id_map_type_value == HDR_CS_ID_MAP_TYPE_IPSEC4_ID )
        ret += "<IPSEC4-ID>";
    else
        ret += "<unknown (" + itoa( cs_id_map_type_value ) + ")>";

    if( cs_id_map_ptr )
    {
        ret += "\n\n";
        ret += cs_id_map_ptr->debug_dump();
        ret += "\n\n";
    }

    return ret;
}

int Mikey_Payload_HDR::data_type() const
{
    return data_type_value;
}
int Mikey_Payload_HDR::v() const
{
    return vvalue;
}
uint32_t Mikey_Payload_HDR::csb_id() const
{
    return csb_id_value;
}
int Mikey_Payload_HDR::cs_id_map_type() const
{
    return cs_id_map_type_value;
}
SRef<Mikey_Cs_Id_Map *> Mikey_Payload_HDR::cs_id_map()
{
    return cs_id_map_ptr;
}

uint8_t Mikey_Payload_HDR::ncs() const
{
    return ncs_value;
}
