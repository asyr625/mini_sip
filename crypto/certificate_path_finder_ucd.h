#ifndef CERTIFICATE_PATH_FINDER_UCD_H
#define CERTIFICATE_PATH_FINDER_UCD_H

#include "sobject.h"
#include "timestamp.h"
#include "cert.h"
#include "certificate_finder.h"
#include "cert_cache_manager.h"

#include <vector>
#include <string>

#ifndef MAX_EFFORT
#define MAX_EFFORT -1
#endif

class Certificate_Path_Finder_Ucd : public SObject
{
public:
    Certificate_Path_Finder_Ucd(SRef<Cert_Cache_Manager*> cm);
    ~Certificate_Path_Finder_Ucd();

    SRef<Certificate_Chain*> find_ucd_path(SRef<Certificate_Chain*> curPath, SRef<Certificate_Set*> & rootCerts, SRef<Certificate*> & toCert);

    SRef<Certificate_Chain*> find_ucd_path(SRef<Certificate*> selfCert, SRef<Certificate*> upCert, SRef<Certificate*> toCert);

    void print_stats(std::string prefix, std::string timeStampFile = "");

private:
    std::vector<SRef<Certificate*> > findCrossCerts	(SRef<Certificate*> curCert, SRef<Certificate*> toCert, int& crossEffort,	int& findEffort);

    std::vector<SRef<Certificate*> > findUpCerts	(SRef<Certificate*> curCert, SRef<Certificate*> toCert,	int& upEffort, 		int& findEffort);

    std::vector<SRef<Certificate*> > findDownCerts	(SRef<Certificate*> curCert, SRef<Certificate*> toCert, int& downEffort, 	int& findEffort);

    std::vector<SRef<Certificate*> > findCerts	(std::vector<std::string> candidates, SRef<Certificate*> curCert, SRef<Certificate*> toCert, int& phaseEffort, int& findEffort);

    std::vector<std::string> candidateCrossPaths(SRef<Certificate*> toCert);

    std::vector<std::string> candidateUpPaths(SRef<Certificate*> curCert, SRef<Certificate*> toCert);

    std::vector<std::string> candidateDownPaths(SRef<Certificate*> curCert, SRef<Certificate*> toCert);

    bool verifyLastPair(std::vector<SRef<Certificate*> > & certList);

    SRef<Certificate_Finder*> cert_finder;
    Certificate_Finder_Stats* stats;
};

#endif // CERTIFICATE_PATH_FINDER_UCD_H
