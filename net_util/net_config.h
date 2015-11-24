#ifndef NET_CONFIG_H
#define NET_CONFIG_H

/* Compilation time configuration */
#ifndef _MSC_VER
//xxx#	include"compilation_config.h"
#else
#	include"compilation_config_w32_wce.h"
#endif

#ifndef LIBMNETUTIL_EXPORTS
# ifdef DLL_EXPORT
#  define LIBMUTIL_IMPORTS
#  define LIBMNETUTIL_EXPORTS
# endif  // DLL_EXPORT
#endif  // LIBMNETUTIL_EXPORTS

#include "my_types.h"

#ifdef _MSC_VER

    #ifndef WIN32
        #define WIN32
    #endif

    #pragma warning (disable: 4251)

    //Warning 4290 is that Microsofts compiler is
    //is ignoring throw() declarations in the class
    //definition.
    #pragma warning (disable: 4290)

    #ifndef LIBMNETUTIL_EXPORTS
        #error Visual Studio is not set up correctly to compile libmutil to a .dll (LIBMNETUTIL_EXPORTS not defined).
    #endif
#endif

//Temporary ... STLPort does not allow addition of errno.h ... but WCEcompat does ...
//So we don't have to repeat this everytime, include errno.h for all files ...
//	anyway, it is just an int :)
//Anyway, while compiling for EVC, it will still trigger some warnings ... ignore them, errno exhists for sure.
#ifdef _WIN32_WCE
#	ifndef _STLP_NATIVE_ERRNO_H_INCLUDED
#		include<wcecompat/errno.h>
#		define _STLP_NATIVE_ERRNO_H_INCLUDED
#	endif
#else
#	include <errno.h>
#endif

/*big/little endian conversion*/
#ifndef _MSC_VER
inline
#endif
static uint16_t U16_AT( void const * _p )
{
    const uint8_t * p = (const uint8_t *)_p;
    return ( ((uint16_t)p[0] << 8) | p[1] );
}

#ifndef _MSC_VER
inline
#endif
static uint32_t U32_AT( void const * _p )
{
    const uint8_t * p = (const uint8_t *)_p;
    return ( ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16)
              | ((uint32_t)p[2] << 8) | p[3] );
}

#ifndef _MSC_VER
inline
#endif
static uint64_t U64_AT( void const * _p )
{
    const uint8_t * p = (const uint8_t *)_p;
    return ( ((uint64_t)p[0] << 56) | ((uint64_t)p[1] << 48)
              | ((uint64_t)p[2] << 40) | ((uint64_t)p[3] << 32)
              | ((uint64_t)p[4] << 24) | ((uint64_t)p[5] << 16)
              | ((uint64_t)p[6] << 8) | p[7] );
}

#if defined WORDS_BIGENDIAN
#   define hton16(i)   ( i )
#   define hton32(i)   ( i )
#   define hton64(i)   ( i )
#   define ntoh16(i)   ( i )
#   define ntoh32(i)   ( i )
#   define ntoh64(i)   ( i )
#else
#   define hton16(i)   U16_AT(&i)
#   define hton32(i)   U32_AT(&i)
#   define hton64(i)   U64_AT(&i)
#   define ntoh16(i)   U16_AT(&i)
#   define ntoh32(i)   U32_AT(&i)
#   define ntoh64(i)   U64_AT(&i)
#endif

#if !defined(HAVE_PIP_ADAPTER_ADDRESSES) \
  || !defined(HAVE_PIP_ADAPTER_DNS_SERVER_ADDRESS)
// Disable use of GetAdaptersAddresses in udns
# define NO_IPHLPAPI
#endif

#endif // NET_CONFIG_H
