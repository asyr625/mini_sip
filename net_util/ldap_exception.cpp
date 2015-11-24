#include "ldap_exception.h"


Ldap_Exception::Ldap_Exception()
{
    msg = "Ldap_Exception";
}

Ldap_Exception::Ldap_Exception(std::string msg)
{
    this->msg = "Ldap_Exception: " + msg;
}

const char* Ldap_Exception::what()const throw()
{
    return msg.c_str();
}

Ldap_Attribute_Not_Found_Exception::Ldap_Attribute_Not_Found_Exception(std::string message)
{
    msg = "Ldap_Attribute_Not_Found_Exception: Could not find attribute " + message;
}

Ldap_Not_Connected_Exception::Ldap_Not_Connected_Exception()
{
    msg = "Ldap_Not_Connected_Exception: Not connected to server";
}

Ldap_Unsupported_Exception::Ldap_Unsupported_Exception(std::string feature)
{
    msg = "Ldap_Unsupported_Exception: Feature " + feature + " not supported";
}
