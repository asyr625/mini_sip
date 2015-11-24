#ifndef NETWORK_EXCEPTION_H
#define NETWORK_EXCEPTION_H

#include "exception.h"


class  Network_Exception : public Exception
{
public:
    Network_Exception( );
    Network_Exception( int error_number );
    virtual ~Network_Exception() throw() {}
    virtual const char *what() const throw();
protected:
    int _error_number;
    std::string _msg;
};

class  Host_Not_Found : public Network_Exception
{
public:
    Host_Not_Found( std::string host_ ):Network_Exception(-1),_host(host_){}
    virtual ~Host_Not_Found()throw() {}
    virtual const char*what();
private:
    std::string _host;
    std::string _msg;
};

class  Unknown_Address_Family : public Network_Exception
{
public:
    Unknown_Address_Family( int error_number );
    virtual const char*what();
};

class  Resolv_Error : public Network_Exception
{
public:
    Resolv_Error( int error_number );
};

class  Connect_Failed : public Network_Exception
{
public:
    Connect_Failed( int error_number );
};

class  Socket_Failed : public Network_Exception
{
public:
    Socket_Failed( int error_number );
};

class  Bind_Failed : public Network_Exception
{
public:
    Bind_Failed( int error_number );
};

class  Send_Failed : public Network_Exception
{
public:
    Send_Failed( int error_number );
};

class  Setsockopt_Failed : public Network_Exception
{
public:
    Setsockopt_Failed( int error_number );
};

class  Listen_Failed : public Network_Exception
{
public:
    Listen_Failed( int error_number );
};

class  Getsockname_Failed : public Network_Exception
{
public:
    Getsockname_Failed( int error_number );
};

#endif // NETWORK_EXCEPTION_H
