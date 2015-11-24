#ifndef TLS_EXCEPTION_H
#define TLS_EXCEPTION_H

#include "network_exception.h"

class Tls_Init_Failed : public Network_Exception
{
public:
    Tls_Init_Failed();
    virtual ~Tls_Init_Failed() throw(){}
    virtual const char *what();
private:
    std::string msg;
};

class Tls_Context_Init_Failed : public Network_Exception
{
public:
    Tls_Context_Init_Failed();
    virtual ~Tls_Context_Init_Failed() throw(){}
    virtual const char*what();
private:
    std::string msg;
};
#endif // TLS_EXCEPTION_H
