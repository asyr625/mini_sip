#include "mikey_payload_t.h"
#include "mikey_exception.h"
#include "string_utils.h"

#include "my_time.h"

#include<assert.h>
#include<string.h>

#ifdef _MSC_VER
#	include<Winsock2.h>
#else
#	include<time.h>
#	include<sys/time.h>
#endif

using namespace std;

Mikey_Payload_T::Mikey_Payload_T()
{
    this->payload_type_value = MIKEYPAYLOAD_T_PAYLOAD_TYPE;
    ts_type_value = T_TYPE_NTP_UTC;

    struct timeval *tv;
    struct timezone *tz;

    tv = new struct timeval;
    tz = new struct timezone;

    gettimeofday( tv, tz );

    uint32_t ts_sec = tv->tv_sec + NTP_EPOCH_OFFSET
            + 60 * tz->tz_minuteswest;
    //10^-6 / 2^-32 = 4294.967296
    uint32_t ts_frac = (uint32_t)( tv->tv_usec * 4294.967296 );

    ts_value = ((uint64_t)ts_sec << 32) | ((uint64_t)ts_frac);
    delete tv;
    delete tz;
}

Mikey_Payload_T::Mikey_Payload_T( int type, uint64_t value )
{
    this->payload_type_value = MIKEYPAYLOAD_T_PAYLOAD_TYPE;

    this->ts_type_value=type;
    this->ts_value = value;
}

Mikey_Payload_T::Mikey_Payload_T( byte_t * start, int lengthLimit )
    : Mikey_Payload( start )
{

    int tsLength;

    this->payload_type_value = MIKEYPAYLOAD_T_PAYLOAD_TYPE;
    if( lengthLimit < 2 )
    {
        throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a T Payload" );
        return;
    }

    set_next_payload_type( start[0] );
    this->ts_type_value= start[1];

    switch( ts_type_value )
    {
    case 0:
        tsLength = 8;
        break;
    case 1:
        tsLength = 8;
        break;
    case 2:
        tsLength = 4;
        break;
    default:
        throw Mikey_Exception_Message_Content(
                    "Unknown type of time stamp" );
        break;

    }
    if( lengthLimit < 2 + tsLength ){
        throw Mikey_Exception_Message_Length_Exception(
                    "Given data is too short to form a T Payload" );
        return;
    }

    switch( tsLength ){
    case 8:
        ts_value =(uint64_t)(start[2]) << 56 | (uint64_t)(start[3]) << 48 |
                                                                       (uint64_t)(start[4]) << 40 |(uint64_t)(start[5]) << 32 |
                                                                                                                           (uint64_t)(start[6]) << 24 |(uint64_t)(start[7]) << 16 |
                                                                                                                                                                               (uint64_t)(start[8]) <<  8 |(uint64_t)(start[9]);
        break;
    case 4:
        ts_value = (uint64_t)(start[2]) << 24 | (uint64_t)(start[3]) << 16 |
                                                                        (uint64_t)(start[4]) <<  8 |(uint64_t)(start[5]);
    }

    end_ptr = start_ptr + 2 + tsLength;

    assert( end_ptr - start_ptr == length() );
}

Mikey_Payload_T::~Mikey_Payload_T()
{
}

int Mikey_Payload_T::length()
{
    return 2 + ( ( ts_type_value == T_TYPE_COUNTER )?4:8 );
}

void Mikey_Payload_T::write_data( byte_t *start, int expectedLength )
{
    int i;
    assert( expectedLength == length() );
    memset( start, 0, expectedLength );
    start[0] = next_payload_type();
    start[1] = (byte_t) ts_type_value;
    switch( ts_type_value )
    {
    case T_TYPE_NTP_UTC:
    case T_TYPE_NTP:
        for( i = 0; i < 8; i++ )
        {
            start[2+i] = (byte_t)((ts_value >> ((7-i)*8)) & 0xFF);
        }
        break;
    case T_TYPE_COUNTER:
        for( i = 0; i < 4; i++ )
        {
            start[2+i] = (byte_t)((ts_value >> ((3-i)*8)) & 0xFF);
        }
    }
}

std::string Mikey_Payload_T::debug_dump()
{
    return "MikeyPayloadT: next_payload=<" +
            itoa( next_payload_type() ) +
            "> tsValue type=<" + itoa( ts_type_value ) +
            "> tsValue_value=<" + itoa( ts_value ) + ">";
}

int64_t Mikey_Payload_T::offset( int type, uint64_t ts )
{
    return this->ts_value - ts;
}

bool Mikey_Payload_T::check_offset( uint64_t max )
{
    struct timeval tv;
    struct timezone tz;
    uint64_t current_time;

    memset( &tv, 0, sizeof( tv ));
    memset( &tz, 0, sizeof( tz ));

    gettimeofday( &tv, &tz );

    //10^-6 / 2^-32 = 4294.967296
    uint32_t ts_frac = (uint32_t)( tv.tv_usec * 4294.967296 );
    uint32_t ts_sec;
    switch( ts_type_value )
    {
    case T_TYPE_NTP_UTC:
        ts_sec = tv.tv_sec + NTP_EPOCH_OFFSET
                + 60 * tz.tz_minuteswest;
        current_time = ((uint64_t)ts_sec << 32)
                | ((uint64_t)ts_frac);
        /*	if( current_time > tsValue ){
                return (current_time - tsValue > max );
            }
            else{
                return (tsValue - current_time > max );
            }*/
        return false;
    case T_TYPE_NTP:
        ts_sec = tv.tv_sec + NTP_EPOCH_OFFSET
                /*+ 60 * tz.tz_minuteswest*/;
        current_time = ((uint64_t)ts_sec << 32)
                | ((uint64_t)ts_frac);
        /*if( current_time > tsValue ){
                return (current_time - tsValue > max );
            }
            else{
                return (tsValue - current_time > max );
            }*/
        return false;
    case T_TYPE_COUNTER:
        throw Mikey_Exception(
                    "Cannot compute a time offset with a counter ts" );
    default:
        throw Mikey_Exception_Message_Content(
                    "Unknown type of time stamp in T payload" );
    }
}

uint64_t Mikey_Payload_T::ts()
{
    return ts_value;
}
