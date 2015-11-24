#include "mikey_cs_id_map.h"

#include "string_utils.h"
#include "mikey_exception.h"

using namespace std;

Mikey_Srtp_Cs::Mikey_Srtp_Cs( uint8_t policyNo, uint32_t _ssrc, uint32_t _roc )
    : policy_no(policyNo), ssrc(_ssrc), roc(_roc)
{
}

Mikey_IPSEC4_Cs::Mikey_IPSEC4_Cs( uint8_t policyNo, uint32_t _spi, uint32_t spiSrcaddr, uint32_t spiDstaddr )
    : policy_no(policyNo), spi(_spi), spi_srcaddr(spiSrcaddr), spi_dstaddr(spiDstaddr)
{
}

Mikey_Cs_Id_Map_Srtp::Mikey_Cs_Id_Map_Srtp()
{

}

Mikey_Cs_Id_Map_Srtp::Mikey_Cs_Id_Map_Srtp( byte_t * data, int length )
{
    if( length % 9 )
    {
        throw Mikey_Exception( "Invalid length of SRTP_ID map info" );
    }

    uint8_t nCs = length / 9;
    uint8_t i;
    uint32_t ssrc, roc;
    byte_t policyNo;

    for( i = 0; i < nCs; i++ )
    {
        policyNo = data[ i*9 ];
        ssrc = (uint32_t)data[ i*9 + 1 ] << 24 | (uint32_t)data[ i*9 + 2 ] << 16 | (uint32_t)data[ i*9 + 3 ] <<  8 | (uint32_t)data[ i*9 + 4 ];
        roc  = (uint32_t)data[ i*9 + 5 ] << 24 | (uint32_t)data[ i*9 + 6 ] << 16 | (uint32_t)data[ i*9 + 7 ] <<  8 | (uint32_t)data[ i*9 + 8 ];
        add_stream( ssrc, roc, policyNo );
    }
}

Mikey_Cs_Id_Map_Srtp::~Mikey_Cs_Id_Map_Srtp()
{
    vector<Mikey_Srtp_Cs *>::iterator i;
    for( i = cs.begin(); i!= cs.end() ; i++ )
        delete *i;
}

int Mikey_Cs_Id_Map_Srtp::length()
{
    return 9 * cs.size();
}

void Mikey_Cs_Id_Map_Srtp::write_data( byte_t * start, int expectedLength )
{
    if( expectedLength < length() )
    {
        throw Mikey_Exception_Message_Length_Exception( "CsSrtpId is too long" );
    }

    int j = 0,k;
    vector<Mikey_Srtp_Cs *>::iterator i;

    for( i = cs.begin(); i != cs.end(); i++ )
    {
        start[ 9*j ] = (*i)->policy_no & 0xFF;
        for( k = 0; k < 4; k++ )
        {
            start[9*j+1+k] = ((*i)->ssrc >> 8*(3-k)) & 0xFF;
        }
        for( k = 0; k < 4; k++ )
        {
            start[9*j+5+k] = ((*i)->roc >> 8*(3-k)) & 0xFF;
        }
        j++;
    }
}

byte_t Mikey_Cs_Id_Map_Srtp::find_cs_id( uint32_t ssrc )
{
    vector<Mikey_Srtp_Cs *>::iterator i;
    uint8_t j = 1;
    for( i = cs.begin(); i != cs.end()  ; i++,j++ )
    {
        if( (*i)->ssrc == ssrc )
        {
            return j;
        }
    }
    return 0;
}

uint32_t Mikey_Cs_Id_Map_Srtp::find_roc( uint32_t ssrc )
{
    vector<Mikey_Srtp_Cs *>::iterator i;
    for( i = cs.begin(); i != cs.end()  ; i++ )
    {
        if( (*i)->ssrc == ssrc )
        {
            return (*i)->roc;
        }
    }
    return 0;
}

byte_t Mikey_Cs_Id_Map_Srtp::findpolicy_no( uint32_t ssrc )
{
    vector<Mikey_Srtp_Cs *>::iterator i;
    for( i = cs.begin(); i != cs.end()  ; i++ )
    {
        if( (*i)->ssrc == ssrc )
        {
            return (*i)->policy_no;
        }
    }
    return 0;
}

void Mikey_Cs_Id_Map_Srtp::add_stream( uint32_t ssrc, uint32_t roc, byte_t policyNo, byte_t csId )
{
    if( csId == 0 )
    {
        cs.push_back( new Mikey_Srtp_Cs( policyNo, ssrc, roc ) );
        return;
    }

    if( csId > cs.size()  )
        return;

    (cs[csId - 1])->ssrc = ssrc;
    (cs[csId - 1])->policy_no = policyNo;
    (cs[csId - 1])->roc = roc;
    return;
}

void Mikey_Cs_Id_Map_Srtp::set_roc( uint32_t roc, uint8_t csId )
{
    if( csId > cs.size() )
        return;

    (cs[ csId - 1 ])->roc = roc;
}

void Mikey_Cs_Id_Map_Srtp::set_ssrc( uint32_t ssrc, uint8_t csId )
{
    if( csId > cs.size() )
    {
        return;
    }
    (cs[ csId - 1 ])->ssrc = ssrc;
}

std::string Mikey_Cs_Id_Map_Srtp::debug_dump()
{
    std::string output = "";
    std::vector<Mikey_Srtp_Cs *>::iterator iCs;
    uint8_t csId = 1;

    for( iCs = cs.begin(); iCs != cs.end(); iCs++, csId++ )
    {
        output += "csId: <" + itoa( csId ) + ">\n";
        output += "   policyNo: <" + itoa( (*iCs)->policy_no ) + ">\n";
        output += "   SSRC: <" + itoa( (*iCs)->ssrc ) + ">\n";
        output += "   ROC: <" + itoa( (*iCs)->roc ) + ">\n";
        output += "\n";
    }
    return output;
}

Mikey_Cs_Id_Map_IPSEC4::Mikey_Cs_Id_Map_IPSEC4()
{
}

Mikey_Cs_Id_Map_IPSEC4::Mikey_Cs_Id_Map_IPSEC4( byte_t * data, int length )
{
    if( length % 13 )
    {
        throw Mikey_Exception( "Invalid length of IPSEC4_ID map info" );
    }

    uint8_t nCs = length / 13;
    uint8_t i;
    uint32_t spi, spiSrcaddr, spiDstaddr;
    byte_t policyNo;

    for( i = 0; i < nCs; i++ )
    {
        policyNo = data[ i*13 ];
        spi = (uint32_t)data[ i*13 + 1 ] << 24 | (uint32_t)data[ i*13 + 2 ] << 16 | (uint32_t)data[ i*13 + 3 ] <<  8 | (uint32_t)data[ i*13 + 4 ];
        spiSrcaddr  = (uint32_t)data[ i*13 + 5 ] << 24 | (uint32_t)data[ i*13 + 6 ] << 16 | (uint32_t)data[ i*13 + 7 ] <<  8 | (uint32_t)data[ i*13 + 8 ];
        spiDstaddr  = (uint32_t)data[ i*13 + 9 ] << 24 | (uint32_t)data[ i*13 + 10 ] << 16 | (uint32_t)data[ i*13 + 11 ] <<  8 | (uint32_t)data[ i*13 + 12 ];
        add_sa( spi, spiSrcaddr, spiDstaddr, policyNo );
    }
}

Mikey_Cs_Id_Map_IPSEC4::~Mikey_Cs_Id_Map_IPSEC4()
{
    list<Mikey_IPSEC4_Cs *>::iterator i;
    for( i = cs.begin(); i!= cs.end() ; i++ )
        delete *i;
}

int Mikey_Cs_Id_Map_IPSEC4::length()
{
    return 13 * cs.size();
}

void Mikey_Cs_Id_Map_IPSEC4::write_data( byte_t * start, int expectedLength )
{
    if( expectedLength < length() )
    {
        throw Mikey_Exception_Message_Length_Exception( "CsIPSEC4Id is too long" );
    }

    int j = 0,k;
    list<Mikey_IPSEC4_Cs *>::iterator i;

    for( i = cs.begin(); i != cs.end(); i++ )
    {
        start[ 13*j ] = (*i)->policy_no & 0xFF;
        for( k = 0; k < 4; k++ )
        {
            start[13*j+1+k] = ((*i)->spi >> 8*(3-k)) & 0xFF;
        }
        for( k = 0; k < 4; k++ ){
            start[13*j+5+k] = ((*i)->spi_srcaddr >> 8*(3-k)) & 0xFF;
        }
        for( k = 0; k < 4; k++ ){
            start[13*j+9+k] = ((*i)->spi_dstaddr >> 8*(3-k)) & 0xFF;
        }
        j++;
    }
}

byte_t Mikey_Cs_Id_Map_IPSEC4::find_cs_id( uint32_t spi, uint32_t spiSrcaddr, uint32_t spiDstaddr )
{
    list<Mikey_IPSEC4_Cs *>::iterator i;
    uint8_t j = 1;
    for( i = cs.begin(); i != cs.end()  ; i++,j++ )
    {
        if( (*i)->spi == spi && (*i)->spi_srcaddr == spiSrcaddr && (*i)->spi_dstaddr == spiDstaddr)
        {
            return j;
        }
    }
    return 0;
}

Mikey_IPSEC4_Cs * Mikey_Cs_Id_Map_IPSEC4::get_cs_idnumber(int number)
{
    list<Mikey_IPSEC4_Cs *>::iterator i;
    int j = 1;
    for( i = cs.begin(); i != cs.end()  ; i++ )
    {
        if(j == number)
            return (*i);
        j++;
    }
    return NULL;
}

byte_t Mikey_Cs_Id_Map_IPSEC4::findpolicy_no( uint32_t spi, uint32_t spiSrcaddr, uint32_t spiDstaddr)
{
    list<Mikey_IPSEC4_Cs *>::iterator i;
    for( i = cs.begin(); i != cs.end()  ; i++ )
    {
        if( (*i)->spi == spi && (*i)->spi_srcaddr == spiSrcaddr && (*i)->spi_dstaddr == spiDstaddr )
        {
            return (*i)->policy_no;
        }
    }
    return 0;
}

void Mikey_Cs_Id_Map_IPSEC4::add_sa( uint32_t spi, uint32_t spiSrcaddr, uint32_t spiDstaddr, byte_t policyNo, byte_t csId )
{
    if( csId == 0 )
    {
        cs.push_back( new Mikey_IPSEC4_Cs( policyNo, spi, spiSrcaddr, spiDstaddr ) );
        return;
    }
    list<Mikey_IPSEC4_Cs *>::iterator i;
    uint8_t j = 1;
    for( i = cs.begin(); i != cs.end() ; i++,j++ )
    {
        if( j == csId )
        {
            (*i)->spi = spi;
            (*i)->policy_no = policyNo;
            (*i)->spi_srcaddr = spiSrcaddr;
            (*i)->spi_dstaddr = spiDstaddr;
        }
    }
    return;
}

std::string Mikey_Cs_Id_Map_IPSEC4::debug_dump()
{
    std::string output = "";
    std::list<Mikey_IPSEC4_Cs *>::iterator iCs;
    uint8_t csId = 1;

    for( iCs = cs.begin(); iCs != cs.end(); iCs++, csId++ )
    {
        output += "csId: <" + itoa( csId ) + ">\n";
        output += "   spi: <" + itoa( (*iCs)->spi ) + ">\n";
        output += "   policyNo: <" + itoa( (*iCs)->policy_no ) + ">\n";
        output += "   Source Addr.: <" + itoa( (*iCs)->spi_srcaddr ) + ">\n";
        output += "   Dest. Addr.: <" + itoa( (*iCs)->spi_dstaddr ) + ">\n";
        output += "\n";
    }
    return output;
}
