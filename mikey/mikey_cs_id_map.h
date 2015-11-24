#ifndef MIKEY_CS_ID_MAP_H
#define MIKEY_CS_ID_MAP_H


#include<vector>
#include<list>
#include "sobject.h"
#include "my_types.h"

#define HDR_CS_ID_MAP_TYPE_SRTP_ID 0
#define HDR_CS_ID_MAP_TYPE_IPSEC4_ID 7


// CS# info for srtp
class Mikey_Srtp_Cs
{
public:
    Mikey_Srtp_Cs(uint8_t policyNo, uint32_t _ssrc, uint32_t _roc=0 );
    uint8_t policy_no;
    uint32_t ssrc;
    uint32_t roc;
};


// CS# info for ipv4 IPSEC
// each CS# is related to an unique combination of spi and spiaddresses.
class Mikey_IPSEC4_Cs
{
public:
    Mikey_IPSEC4_Cs(uint8_t policyNo, uint32_t _spi, uint32_t spiSrcaddr, uint32_t spiDstaddr );
    uint8_t policy_no;
    uint32_t spi;
    uint32_t spi_srcaddr;
    uint32_t spi_dstaddr;
};

class Mikey_Cs_Id_Map : public SObject
{
public:
    virtual int length() = 0;
    virtual void write_data( byte_t * start, int expectedLength ) = 0;
    virtual std::string debug_dump() = 0;
    virtual std::string get_mem_object_type() const { return "MikeyCsIdMap"; }
};

// Srtp map
class Mikey_Cs_Id_Map_Srtp : public Mikey_Cs_Id_Map
{
public:
    Mikey_Cs_Id_Map_Srtp();
    Mikey_Cs_Id_Map_Srtp( byte_t * data, int length );
    ~Mikey_Cs_Id_Map_Srtp();

    virtual int length();
    virtual void write_data( byte_t * start, int expectedLength );

    virtual std::string debug_dump();

    byte_t find_cs_id( uint32_t ssrc );
    uint32_t find_roc( uint32_t ssrc );
    byte_t findpolicy_no( uint32_t ssrc );
    void add_stream( uint32_t ssrc, uint32_t roc=0, byte_t policyNo=0, byte_t csId=0 );

    void set_roc( uint32_t roc, uint8_t csId );
    void set_ssrc( uint32_t ssrc, uint8_t csId );

private:
    std::vector<Mikey_Srtp_Cs *> cs;
};

// ipv4 IPSEC map
class Mikey_Cs_Id_Map_IPSEC4 : public Mikey_Cs_Id_Map
{
public:
    Mikey_Cs_Id_Map_IPSEC4();
    Mikey_Cs_Id_Map_IPSEC4( byte_t * data, int length );
    ~Mikey_Cs_Id_Map_IPSEC4();

    virtual int length();
    virtual void write_data( byte_t * start, int expectedLength );
    virtual std::string debug_dump();

    Mikey_IPSEC4_Cs * get_cs_idnumber(int number);
    byte_t find_cs_id( uint32_t spi, uint32_t spiSrcaddr, uint32_t spiDstaddr );
    byte_t findpolicy_no( uint32_t spi, uint32_t spiSrcaddr, uint32_t spiDstaddr);
    void add_sa( uint32_t spi, uint32_t spiSrcaddr, uint32_t spiDstaddr, byte_t policyNo=0, byte_t csId = 0 );

private:
    std::list<Mikey_IPSEC4_Cs *> cs;
};

#endif // MIKEY_CS_ID_MAP_H
