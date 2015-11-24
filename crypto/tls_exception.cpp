#include "tls_exception.h"


Tls_Init_Failed::Tls_Init_Failed()
    : Network_Exception()
{

}
const char *Tls_Init_Failed::what()
{
    msg = "TLS initialization failed.";
    return msg.c_str();
}

Tls_Context_Init_Failed::Tls_Context_Init_Failed()
    : Network_Exception()
{

}

const char* Tls_Context_Init_Failed::what()
{
    msg = "TLS context initialization failed.";
    return msg.c_str();
}
