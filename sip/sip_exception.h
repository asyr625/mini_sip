#ifndef SIP_EXCEPTION_H
#define SIP_EXCEPTION_H

#include "exception.h"

class Sip_Exception : public Exception
{
public:
    Sip_Exception(const char *desc):Exception(desc){}
};

class Sip_Exception_Invalid_Message : public Sip_Exception
{
public:
    Sip_Exception_Invalid_Message(const char *desc):Sip_Exception(desc){}
};

class Sip_Exception_Invalid_Start : public Sip_Exception
{
public:
    Sip_Exception_Invalid_Start(const char *desc): Sip_Exception(desc){}
};

class Sip_Exception_Invalid_URI : public Sip_Exception_Invalid_Message
{
public:
    Sip_Exception_Invalid_URI(const char *desc) : Sip_Exception_Invalid_Message(desc){}
};

#endif // SIP_EXCEPTION_H
