#include <string.h>
#include "network_exception.h"


Unknown_Address_Family::Unknown_Address_Family( int error_number ): Network_Exception(error_number) {}

Resolv_Error::Resolv_Error( int error_number ): Network_Exception(error_number) {}

Connect_Failed::Connect_Failed( int error_number ): Network_Exception(error_number) {}

Socket_Failed::Socket_Failed( int error_number ): Network_Exception(error_number) {}

Bind_Failed::Bind_Failed( int error_number ): Network_Exception(error_number) {}

Send_Failed::Send_Failed( int error_number ): Network_Exception(error_number) {}

Setsockopt_Failed::Setsockopt_Failed( int error_number ): Network_Exception(error_number) {}

Listen_Failed::Listen_Failed( int error_number ): Network_Exception(error_number) {}

Getsockname_Failed::Getsockname_Failed( int error_number ): Network_Exception(error_number) {}


Network_Exception::Network_Exception() :_error_number(0) {}

Network_Exception::Network_Exception( int error_number ) : _error_number(error_number)
{
#ifdef WIN32
    _msg = string( strerror( _error_number ));
#else
    char buf[256];
    buf[0]=0;
    #if defined(GNU)
    int ret = strerror_r( _error_number, buf, sizeof( buf ) );
    _msg = string( buf );

    #else
    if (strerror_r( _error_number, buf, sizeof( buf ) ))
        _msg="Unknown error";
    else
        _msg = std::string( buf );
    #endif
#endif
}


const char* Network_Exception::what()const throw()
{
    return _msg.c_str();
}


const char* Host_Not_Found::what()
{
    _msg = "Host "+_host+" not found.";
    return _msg.c_str();
}


const char* Unknown_Address_Family::what()
{
    _msg = "Unknown address family: " + _error_number;
    return _msg.c_str();
}
