#ifndef MIKEY_EXCEPTION_H
#define MIKEY_EXCEPTION_H

#include "mikey_message.h"

#include "sobject.h"
#include "exception.h"

class Mikey_Exception : public Exception
{
public:
    Mikey_Exception(const char* message) : Exception(message) {}
    virtual ~Mikey_Exception()throw (){}
};

class Mikey_Exception_Uninitialized: public Mikey_Exception
{
public:
    Mikey_Exception_Uninitialized(const char* msg) : Mikey_Exception(msg) {}
    virtual ~Mikey_Exception_Uninitialized() throw() {}
};

class Mikey_Exception_Message_Content: public Mikey_Exception
{
public:
    Mikey_Exception_Message_Content(const char* msg): Mikey_Exception(msg) {}

    Mikey_Exception_Message_Content(SRef<Mikey_Message *> errMsg, const char* msg="")
        :Mikey_Exception(msg),error_message_value(errMsg) {}

    virtual ~Mikey_Exception_Message_Content()throw() {}

    SRef<Mikey_Message *> error_message()
    {
        return error_message_value;
    }

private:
    SRef<Mikey_Message *> error_message_value;
};



class Mikey_Exception_Message_Length_Exception: public Mikey_Exception
{
public:
    Mikey_Exception_Message_Length_Exception(const char* msg): Mikey_Exception(msg) {}
    virtual ~Mikey_Exception_Message_Length_Exception() throw() {}
};


class Mikey_Exception_Null_Pointer_Exception : public Mikey_Exception
{
public:
    Mikey_Exception_Null_Pointer_Exception(const char* msg) : Mikey_Exception(msg){}
    virtual ~Mikey_Exception_Null_Pointer_Exception() throw() {}
};


class Mikey_Exception_Authentication : public Mikey_Exception
{
public:
    Mikey_Exception_Authentication(const char* msg): Mikey_Exception(msg) {}
    virtual ~Mikey_Exception_Authentication() throw() {}
};

class Mikey_Exception_Unacceptable : public Mikey_Exception
{
public:
    Mikey_Exception_Unacceptable(const char* msg) : Mikey_Exception(msg) {}
    virtual ~Mikey_Exception_Unacceptable() throw() {}
};

class Mikey_Exception_Unimplemented : public Mikey_Exception
{
public:
    Mikey_Exception_Unimplemented(const char* msg) : Mikey_Exception(msg) {}
    virtual ~Mikey_Exception_Unimplemented() throw() {}
};

#endif // MIKEY_EXCEPTION_H
