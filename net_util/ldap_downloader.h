#ifndef LDAP_DOWNLOADER_H
#define LDAP_DOWNLOADER_H

#include "sobject.h"
#include "downloader.h"
#include "ldap_entry.h"
#include "ldap_url.h"
#include "ldap_connection.h"
#include "ldap_exception.h"

#include <string>
#include <vector>
#include <map>

class Ldap_Downloader : public Downloader
{
public:
    Ldap_Downloader(std::string originalUrl);
    char* get_chars(int *length);

    std::vector<std::string> save_to_files(std::string attr, std::string filenameBase, bool onlyFirst = false);

    std::string get_string(std::string attr) throw (Ldap_Attribute_Not_Found_Exception, Ldap_Exception);

    SRef<Ldap_Entry_Binary_Value*> get_binary(std::string attr) throw (Ldap_Attribute_Not_Found_Exception, Ldap_Exception);


    virtual std::string get_mem_object_type() const {return "LdapDownloader";}
private:
    Ldap_Url url;
    SRef<Ldap_Connection*> conn;
    std::vector<SRef<Ldap_Entry*> > entries;
    bool is_loaded;

    // Functions
    void fetch();
    std::string next_filename(std::string baseName, int num);
};

#endif // LDAP_DOWNLOADER_H
