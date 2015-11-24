#ifndef LDAP_DIRECTORY_LOCATOR_H
#define LDAP_DIRECTORY_LOCATOR_H

#include "sobject.h"

class Ldap_Directory_Locator : public SObject
{
public:
    static std::string find_dns_srv();
    static std::string find_dns_alias();
    static std::string find_user_config();
    static std::string find_user_cache();
    static std::string find();
};

#endif // LDAP_DIRECTORY_LOCATOR_H
