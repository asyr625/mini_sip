#ifndef LDAP_EXCEPTION_H
#define LDAP_EXCEPTION_H

#include "exception.h"

class Ldap_Exception : public Exception
{
public:
    Ldap_Exception();
    Ldap_Exception(std::string message);
    virtual ~Ldap_Exception() throw() {}
    virtual const char *what() const throw();
protected:
    std::string msg;
};

class Ldap_Attribute_Not_Found_Exception : public Ldap_Exception
{
public:
    Ldap_Attribute_Not_Found_Exception(std::string message);
};

class Ldap_Not_Connected_Exception : public Ldap_Exception
{
public:
    Ldap_Not_Connected_Exception();
};

class Ldap_Unsupported_Exception : public Ldap_Exception
{
public:
    Ldap_Unsupported_Exception(std::string feature);
};

#endif // LDAP_EXCEPTION_H
