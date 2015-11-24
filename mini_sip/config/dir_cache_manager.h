#ifndef DIR_CACHE_MANAGER_H
#define DIR_CACHE_MANAGER_H

#include "sobject.h"
#include "cache_item.h"
#include "directory_set.h"
#include "directory_set_item.h"

#include <string>
#include <map>

#define CACHEMANAGER_CERTSET_ROOTCAS "rootcas"
#define CACHEMANAGER_DIRSET_MAIN "main"

class Dir_Cache_Manager : public SObject
{
public:
    SRef<Directory_Set_Item*> find_directory(const std::string domain, const std::string defaultSet = "");
    SRef<Directory_Set*> get_directory_set(std::string key);

    std::string add_directory(SRef<Directory_Set_Item*> dirItem, std::string setKey);
    std::string add_directory_ldap(std::string url, std::string subTree, const std::string setKey);

private:
    std::string get_new_directory_key() const;

    std::map<std::string, SRef<Directory_Set*> > directory_sets;
};

#endif // DIR_CACHE_MANAGER_H
