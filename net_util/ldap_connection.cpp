#include "net_config.h"
#include "ldap_connection.h"

#ifdef ENABLE_LDAP
#ifdef _MSC_VER
#include <windows.h>
#include <winldap.h>
#else
//FIXME: We use a depricated version of the API.
#define LDAP_DEPRECATED 1
#include <ldap.h>
#endif
#endif

#ifndef LDAP_OPT_SUCCESS
#define LDAP_OPT_SUCCESS 0
#endif

Ldap_Connection::Ldap_Connection(std::string host, int32_t port)
{
#ifdef ENABLE_LDAP
    init(host, port, NULL);
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

Ldap_Connection::Ldap_Connection(std::string host, int32_t port, SRef<Ldap_Credentials*> cred)
{
#ifdef ENABLE_LDAP
    init(host, port, cred);
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

Ldap_Connection::Ldap_Connection(std::string host)
{
#ifdef ENABLE_LDAP
    init(host, LDAP_PORT, NULL);
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

Ldap_Connection::Ldap_Connection(std::string host, SRef<Ldap_Credentials*> cred)
{
#ifdef ENABLE_LDAP
    init(host, LDAP_PORT, cred);
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

Ldap_Connection::~Ldap_Connection()
{
#ifdef ENABLE_LDAP
    disconnect();
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

void Ldap_Connection::init(std::string host, int32_t aPort, SRef<Ldap_Credentials*> aCred)
{
#ifdef ENABLE_LDAP
    hostname = host;
    port = aPort;
    cred = aCred;
    ld = NULL;
    is_bound = false;
    set_credentials(cred);
    try {
        connect();
    } catch (Ldap_Exception & /*e*/ ) {
        //std::cerr << e.what() << std::endl;
    }
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

void Ldap_Connection::connect() throw (Ldap_Exception)
{
#ifdef ENABLE_LDAP
    int auth_method = LDAP_AUTH_SIMPLE;
    int desired_version = LDAP_VERSION3;

    if (is_connected())
    {
        disconnect();
    }

    // Initialize LDAP library and open connection
    if ((ld = ldap_init((char*)hostname.c_str(), port)) == NULL )
    {
        throw Ldap_Exception("Could not connect to server");
    }

    // Set protocol version
    if (ldap_set_option((LDAP*)ld, LDAP_OPT_PROTOCOL_VERSION, &desired_version) != LDAP_OPT_SUCCESS)
    {
        throw Ldap_Exception("Could not set connection options after setting up connection");
    }

    // Create LDAP bind
    const char *user = NULL;
    const char *pass = NULL;

    if (cred) {
        user = cred->username.c_str();
        pass = cred->password.c_str();
    }

    if (ldap_bind_s( (LDAP*)ld,
#ifdef _MSC_VER
            (const PCHAR)
#endif
            user,
#ifdef _MSC_VER
            (const PCHAR)
#endif
            pass,
            auth_method) != LDAP_SUCCESS) {
        throw Ldap_Exception("Could not bind to connected server");
    }

    is_bound = true;
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

bool Ldap_Connection::is_connected(bool alsoCheckBind )
{
#ifdef ENABLE_LDAP
    if (NULL == ld)
        return false;
    if (alsoCheckBind && !is_bound)
        return false;
    return true;
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

bool Ldap_Connection::disconnect()
{
#ifdef ENABLE_LDAP
    if (is_bound) {
        return ldap_unbind((LDAP*)ld) != 0;
    }else
        return false;
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

std::vector<SRef<Ldap_Entry*> > Ldap_Connection::find(std::string baseDn, std::string query, std::vector<std::string> & attrs) throw (Ldap_Not_Connected_Exception, Ldap_Exception)
{
#ifdef ENABLE_LDAP
    return find(baseDn, query, attrs, LDAP_SCOPE_SUBTREE);
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

std::vector<SRef<Ldap_Entry*> > Ldap_Connection::find(std::string baseDn, std::string query, std::vector<std::string> & attrs, int scope) throw (Ldap_Not_Connected_Exception, Ldap_Exception)
{
#ifdef ENABLE_LDAP
    LDAPMessage* msg;
    std::vector<SRef<Ldap_Entry*> > entries = std::vector<SRef<Ldap_Entry*> >();
    char **searchAttrs = new char*[attrs.size()+1];
    LDAPMessage* entry;
    int i=0;

    // Test if client is connected
    if (!is_connected())
    {
        throw Ldap_Not_Connected_Exception();
    }

    // Convert vector of C++ strings to array of C-style strings
    for(i = 0; i < (int)attrs.size(); i++ )
    {
        searchAttrs[i] = const_cast<char*>(attrs.at(i).c_str());
    }

    // OpenLDAP requires that the last entry is NULL (my guess is to avoid needing an additional "length variable")
    searchAttrs[attrs.size()] = NULL;

    // Send query (note that it is blocking!)
    if (ldap_search_s(
            (LDAP*)ld,
#ifdef _MSC_VER
            (PCHAR)
#endif
            baseDn.c_str(),
            scope,
#ifdef _MSC_VER
            (PCHAR)
#endif
            query.c_str(),
            searchAttrs,
            0,
            &msg) != LDAP_SUCCESS)
    {
        delete []searchAttrs;
        // Return empty list
        throw Ldap_Exception("LdapException: Could not execute query");
    }


    // Push each returned object (entry) onto the result vector
    for( entry = ldap_first_entry((LDAP*)ld, msg);entry != NULL; entry = ldap_next_entry((LDAP*)ld,entry))
    {
        // Parse the returned entry directly (don't wait until the user actually "needs" it)
        entries.push_back(SRef<Ldap_Entry*>(new Ldap_Entry((LDAP*)ld, entry)));

    }

    // Clear up some memory
    ldap_msgfree(msg);

    delete []searchAttrs;

    return std::vector<SRef<Ldap_Entry*> >(entries);
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

std::string Ldap_Connection::get_base_dn() throw (Ldap_Not_Connected_Exception, Ldap_Unsupported_Exception)
{
#ifdef ENABLE_LDAP

    if (!is_connected())
    {
        throw Ldap_Not_Connected_Exception();
    }

    std::vector<SRef<Ldap_Entry*> > entries;
    std::vector<std::string> attrs;

    attrs.push_back("namingContexts");
    entries = find("", "(objectclass=*)", attrs, LDAP_SCOPE_BASE);

    if (0 == entries.size())
    {
        throw Ldap_Unsupported_Exception("Retrieving base DN");
    }
    else
    {
        try {
            return entries.at(0)->get_attr_value_string("namingContexts");
        } catch (Ldap_Attribute_Not_Found_Exception & /*e*/ ) {
            throw Ldap_Unsupported_Exception("Retrieving base DN");
        }
    }
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

void Ldap_Connection::set_credentials(SRef<Ldap_Credentials*> cred)
{
#ifdef ENABLE_LDAP
    this->cred = cred;
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}

SRef<Ldap_Credentials*>	Ldap_Connection::get_credentials()
{
#ifdef ENABLE_LDAP
    return cred;
#else
    throw Ldap_Exception("LDAP support not enabled");
#endif
}
