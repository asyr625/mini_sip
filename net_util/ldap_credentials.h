#ifndef LDAP_CREDENTIALS_H
#define LDAP_CREDENTIALS_H

#include <string>
#include "sobject.h"

class Ldap_Credentials : public SObject
{
public:
    Ldap_Credentials(std::string usr,std::string pw)
        : username(usr), password(pw)
    {
    }
    std::string username;
    std::string password;
};

#endif // LDAP_CREDENTIALS_H
