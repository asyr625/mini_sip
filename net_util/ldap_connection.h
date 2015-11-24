#ifndef LDAP_CONNECTION_H
#define LDAP_CONNECTION_H

#include "ldap_entry.h"
#include "ldap_exception.h"
#include "ldap_credentials.h"

class Ldap_Connection : public SObject
{
public:
    Ldap_Connection(std::string host, int32_t port);
    Ldap_Connection(std::string host, int32_t port, SRef<Ldap_Credentials*> cred);
    Ldap_Connection(std::string host);
    Ldap_Connection(std::string host, SRef<Ldap_Credentials*> cred);
    ~Ldap_Connection();

    void connect() throw (Ldap_Exception);
    bool is_connected(bool alsoCheckBind = false);
    bool disconnect();

    std::vector<SRef<Ldap_Entry*> > find(std::string baseDn, std::string query, std::vector<std::string> & attrs) throw (Ldap_Not_Connected_Exception, Ldap_Exception);
    std::vector<SRef<Ldap_Entry*> > find(std::string baseDn, std::string query, std::vector<std::string> & attrs, int scope) throw (Ldap_Not_Connected_Exception, Ldap_Exception);

    std::string get_base_dn() throw (Ldap_Not_Connected_Exception, Ldap_Unsupported_Exception);

    void set_credentials(SRef<Ldap_Credentials*> cred);

    SRef<Ldap_Credentials*>	get_credentials();

protected:
    void init(std::string host, int32_t port, SRef<Ldap_Credentials*> cred);

private:
    void*			ld;
    std::string 	hostname;
    int32_t			port;
    SRef<Ldap_Credentials*> cred;
    bool			is_bound;
};

#endif // LDAP_CONNECTION_H
