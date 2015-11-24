#include "key_validity.h"

#include "mikey_exception.h"
#include "mikey_defs.h"
#include "string_utils.h"

#include<assert.h>
#include<string.h>

using namespace std;

Key_Validity::Key_Validity()
{
    type_value = KEYVALIDITY_NULL;
}

Key_Validity::Key_Validity( const Key_Validity& )
{
    type_value = KEYVALIDITY_NULL;
}

Key_Validity::~Key_Validity()
{
}

void Key_Validity::operator =( const Key_Validity& )
{
    type_value = KEYVALIDITY_NULL;
}

int Key_Validity::length()
{
    return 0;
}

int Key_Validity::type()
{
    return type_value;
}

void Key_Validity::write_data( byte_t * start, int expectedLength )
{
}

std::string Key_Validity::debug_dump()
{
    return "KeyValidityNull";
}


Key_Validity_SPI::Key_Validity_SPI()
    : spi_length(0), spi_ptr(NULL)
{
    type_value = KEYVALIDITY_SPI;
}

Key_Validity_SPI::Key_Validity_SPI( const Key_Validity_SPI& source)
{
    type_value = KEYVALIDITY_SPI;
    spi_length = source.spi_length;
    spi_ptr = new byte_t[ spi_length ];
    memcpy( this->spi_ptr, source.spi_ptr, spi_length );
}

Key_Validity_SPI::Key_Validity_SPI( byte_t * rawData, int lengthLimit )
{
    if( lengthLimit < 1 )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a KeyValiditySPI" );
    }

    spi_length = rawData[0];

    if( lengthLimit < 1 + spi_length )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a KeyValiditySPI" );
    }

    spi_ptr = new byte_t[ spi_length ];
    memcpy( spi_ptr, &rawData[1], spi_length );
}

Key_Validity_SPI::Key_Validity_SPI( int length, byte_t * spi )
{
    this->spi_ptr = new byte_t[ length ];
    memcpy( this->spi_ptr, spi, length );
    this->spi_length = length;
}

Key_Validity_SPI::~Key_Validity_SPI()
{
    if( spi_ptr )
        delete [] spi_ptr;
    return;
}

void Key_Validity_SPI::operator =( const Key_Validity_SPI& source)
{
    if( spi_ptr ){
        delete [] spi_ptr;
    }

    spi_length = source.spi_length;
    spi_ptr = new byte_t[ spi_length ];
    memcpy( spi_ptr, source.spi_ptr, spi_length );
}

int Key_Validity_SPI::length()
{
    return spi_length + 1;
}

void Key_Validity_SPI::write_data( byte_t * start, int expectedLength )
{
    assert( expectedLength == length() );
    start[0] = spi_length;
    memcpy( &start[1], spi_ptr, spi_length );
}

std::string Key_Validity_SPI::debug_dump()
{
    return (const char *)("Key_Validity_SPI: spi=<") +
            bin_to_hex( spi_ptr, spi_length );
}

Key_Validity_Interval::Key_Validity_Interval()
    :vf_length(0), vf(NULL), vt_length(0), vt(NULL)
{
    type_value = KEYVALIDITY_INTERVAL;
}

Key_Validity_Interval::Key_Validity_Interval( const Key_Validity_Interval& source)
{
    type_value = KEYVALIDITY_INTERVAL;
    vf_length = source.vf_length;
    vf = new byte_t[ vf_length ];
    memcpy( vf, source.vf, vf_length );
    vt_length = source.vt_length;
    vt = new byte_t[ vt_length ];
    memcpy( vt, source.vt, vt_length );
}

Key_Validity_Interval::Key_Validity_Interval( byte_t * raw_data, int length_limit )
{
    if( length_limit < 2 )
        throw Mikey_Exception_Message_Length_Exception( "Given data is too short to form a KeyValidityInterval" );
    vf_length = raw_data[0];
    if( length_limit < 2 + vf_length )
        throw Mikey_Exception_Message_Length_Exception( "Given data is too short to form a KeyValidityInterval" );

    vf = new byte_t[ vf_length ];
    memcpy( vf, &raw_data[1], vf_length );
    vt_length = raw_data[vf_length + 1];
    if( length_limit < 2 + vf_length + vt_length )
        throw Mikey_Exception_Message_Length_Exception( "Given data is too short to form a KeyValidityInterval" );

    vt = new byte_t[ vt_length ];
    memcpy( vt, &raw_data[vf_length + 2], vt_length );
}

Key_Validity_Interval::Key_Validity_Interval( int vfLength, byte_t * vf, int vtLength, byte_t * vt )
{
    this->vf = new byte_t[ vfLength ];
    memcpy( this->vf, vf, vfLength );
    this->vf_length = vfLength;
    this->vt = new byte_t[ vtLength ];
    memcpy( this->vt, vt, vtLength );
    this->vt_length = vtLength;
}

Key_Validity_Interval::~Key_Validity_Interval()
{
    if( vf )
        delete [] vf;
    if( vt )
        delete [] vt;
}

void Key_Validity_Interval::operator =( const Key_Validity_Interval& source)
{
    type_value = KEYVALIDITY_INTERVAL;
    if( vf ){
        delete [] vf;
    }
    if( vt ){
        delete [] vt;
    }

    vf_length = source.vf_length;
    vf = new byte_t[ vf_length ];
    memcpy( vf, source.vf, vf_length );
    vt_length = source.vt_length;
    vt = new byte_t[ vt_length ];
    memcpy( vt, source.vt, vt_length );
}

int Key_Validity_Interval::length()
{
    return vt_length + vf_length + 3;
}

void Key_Validity_Interval::write_data(byte_t * start, int expectedLength)
{
    assert( expectedLength == length() );
    start[0] = vf_length;
    memcpy( &start[1], vf, vf_length );
    start[ 1+vf_length ] = vt_length;
    memcpy( &start[2+vf_length], vt, vt_length );
}

std::string Key_Validity_Interval::debug_dump()
{
    return "Key_Validity_Interval: vf=<" + bin_to_hex( vf, vf_length )+
        "> vt=<" + bin_to_hex( vt, vt_length );
}
