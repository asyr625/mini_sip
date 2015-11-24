#include "downloader.h"
#include "file_downloader.h"
#include "http_downloader.h"


#ifdef ENABLE_LDAP
#include "ldap_downloader.h"
#endif

SRef<Downloader*> Downloader::create(std::string const uri, SRef<Stream_Socket*> conn)
{
    size_t pos = uri.find("://");
    if (std::string::npos != pos)
    {
        std::string protocol = uri.substr(0, pos);
        if( protocol == "http" )
            return new Http_Downloader(uri, conn);
        else if( protocol == "file" )
            return new File_Downloader(uri);
#ifdef ENABLE_LDAP
        else if( protocol == "ldap" )
            return new Ldap_Downloader(uri);
#endif
    }
    return NULL;
}
