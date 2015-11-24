#ifndef MIKEY_PAYLOAD_HDR_H
#define MIKEY_PAYLOAD_HDR_H

#include "mikey_cs_id_map.h"
#include "mikey_payload.h"

#include<list>

#define HDR_DATA_TYPE_PSK_INIT    0
#define HDR_DATA_TYPE_PSK_RESP    1
#define HDR_DATA_TYPE_PK_INIT     2
#define HDR_DATA_TYPE_PK_RESP     3
#define HDR_DATA_TYPE_DH_INIT     4
#define HDR_DATA_TYPE_DH_RESP     5
#define HDR_DATA_TYPE_ERROR       6
#define HDR_DATA_TYPE_DHHMAC_INIT 7
#define HDR_DATA_TYPE_DHHMAC_RESP 8
#define HDR_DATA_TYPE_RSA_R_INIT  9
#define HDR_DATA_TYPE_RSA_R_RESP 10

#define HDR_PRF_MIKEY_1   0
#define HDR_PRF_MIKEY_256 1
#define HDR_PRF_MIKEY_384 2
#define HDR_PRF_MIKEY_512 3

#define HDR_CS_ID_MAP_TYPE_SRTP_ID 0

#define MIKEYPAYLOAD_HDR_PAYLOAD_TYPE (-1)

class Mikey_Payload_HDR : public Mikey_Payload
{
public:
    Mikey_Payload_HDR( byte_t *start_of_header, int lengthLimit );
    Mikey_Payload_HDR( int data_type, int V, int PRF_func, uint32_t CSB_id, int n_cs, int map_type, SRef<Mikey_Cs_Id_Map *> map );


    ~Mikey_Payload_HDR();
    virtual int length();

    virtual void write_data( byte_t * start, int expectedLength );
    virtual std::string debug_dump();

    int data_type() const;
    int v() const;
    uint32_t csb_id() const;
    int cs_id_map_type() const;
    SRef<Mikey_Cs_Id_Map *> cs_id_map();
    uint8_t ncs() const;

private:
    int version;
    int data_type_value;
    int vvalue;
    int prf_func;
    uint32_t csb_id_value;
    int ncs_value;
    int cs_id_map_type_value;
    SRef<Mikey_Cs_Id_Map *> cs_id_map_ptr;
};

#endif // MIKEY_PAYLOAD_HDR_H
