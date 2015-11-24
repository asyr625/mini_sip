#ifndef CACHE_MANAGER_H
#define CACHE_MANAGER_H

#include <string>
#include <map>
#include <list>
#include <vector>

#include "sobject.h"

#include "cert.h"
#include "cache_item.h"
#include "directory_set.h"
#include "directory_set_item.h"

#define CACHEMANAGER_CERTSET_DOWNLOADED "downloadedcerts"
#define CACHEMANAGER_CERTSET_ROOTCAS "rootcas"
#define CACHEMANAGER_DIRSET_MAIN "main"

class CertFindSettings : public SObject
{
public:
    CertFindSettings(const std::string s, const std::string i) : searchText(s), issuer(i) {}
    //CertFindSettings();
    std::string searchText;
    std::string issuer;
};

class Cert_Cache_Manager : public SObject
{
public:
    Cert_Cache_Manager();
    SRef<Certificate*> find_certificate();

    SRef<Directory_Set_Item*> find_directory(const std::string domain, const std::string defaultSet = "");

    SRef<Directory_Set*> get_directory_set(std::string key);

    std::string add_directory(SRef<Directory_Set_Item*> dirItem, std::string setKey);

    std::string add_directory_ldap(std::string url, std::string subTree, const std::string setKey);

    std::vector<SRef<Certificate*> > find_certificates(const std::string searchText, const std::string issuer, const std::string defaultSet = "");

    void add_certificate_set(const SRef<Certificate_Set*> certSet, const std::string setKey);
    std::string add_certificate(const SRef<Certificate*> certItem, std::string setKey);

    bool find_certs_failed_before(const std::string searchText, const std::string issuer);
    void add_find_certs_failed(const std::string searchText, const std::string issuer);

private:
    std::string get_new_directory_set_key() const;
    std::string get_new_certificate_set_key() const;

    std::map<std::string, SRef<Directory_Set*> > directorySets;
    std::map<std::string, SRef<Certificate_Set*> > certificateSets;

    std::list<SRef<CertFindSettings*> > failedCertSearches;
};

#endif // CACHE_MANAGER_H
