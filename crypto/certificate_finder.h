#ifndef CERTIFICATE_FINDER_H
#define CERTIFICATE_FINDER_H

#include "sobject.h"
#include "timestamp.h"
#include "cert.h"
#include "downloader.h"
#include "ldap_downloader.h"
#include "cert_cache_manager.h"

#include <string>
#include <vector>

#ifndef MAX_EFFORT
#define MAX_EFFORT -1
#endif


#define CERTCACHEUSE_NONE 0
/* Cache only certificates in the path, no irrelevant certificates */
#define CERTCACHEUSE_LOW 1
/* Cache all downloaded certificates */
#define CERTCACHEUSE_NORMAL 2
/* The constant used to determine the cache level */
#define USE_CERTIFICATE_CACHE CERTCACHEUSE_NORMAL

#define USE_FINDCERTSFAILED_CACHE 1


class Certificate_Finder_Stats : public SObject
{
public:
    Certificate_Finder_Stats() : ldapQueries (0),
        ldapQueriesNoResult (0),
        ldapQueriesNoDirectory (0),
        ldapCertsDownloaded (0),
        dnsQueries (0),
        dnsQueriesNoResult (0),
        dnsSrvQueries (0),
        dnsSrvQueriesNoResult (0),
        cacheQueries (0),
        cacheQueriesNoResult (0),
        certsProcessed (0),
        certsUseful (0)
    { }

    int ldapQueries;
    int ldapQueriesNoResult;
    int ldapQueriesNoDirectory;
    int ldapCertsDownloaded;
    int dnsQueries;
    int dnsQueriesNoResult;
    int dnsSrvQueries;
    int dnsSrvQueriesNoResult;
    int cacheQueries;
    int cacheQueriesNoResult;
    int certsProcessed;
    int certsUseful;
    Timestamp ts;
};

class Certificate_Finder : public SObject
{
public:
    Certificate_Finder();
    Certificate_Finder(SRef<Cert_Cache_Manager*> cm);

    void set_stats_object(Certificate_Finder_Stats * stats)
    {
        this->stats = stats;
    }

    std::vector<SRef<Certificate*> > find(const std::string subjectUri, SRef<Certificate*> curCert, int & effort, const bool typeCrossCert);

    void set_auto_cache_certs(const bool value);

    bool get_auto_cache_certs() const;
    std::string get_subject_domain(SRef<Certificate*> cert);

private:
    std::vector<SRef<Certificate*> > download_from_ldap(const Ldap_Url & url, const std::string sipUri, const std::string issuer, const bool typeCrossCert);

    SRef<Cert_Cache_Manager*> cache_manager;
    bool autoAddToCache;

    Certificate_Finder_Stats * stats;
};

#endif // CERTIFICATE_FINDER_H
