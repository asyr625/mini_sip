#include "dir_cache_manager.h"
#include "string_utils.h"

SRef<Directory_Set_Item*> Dir_Cache_Manager::find_directory(const std::string domain, const std::string defaultSet)
{
    std::vector<SRef<Directory_Set_Item*> > res;
    if (defaultSet.length() == 0)
    {
        // Scan all directory sets
        //std::map<const std::string, SRef<DirectorySet*> >::iterator i = directorySets.begin();
        //while (i != directorySets.end()) {
        std::map<std::string, SRef<Directory_Set*> >::iterator i = directory_sets.begin();
        for (; i != directory_sets.end(); i++)
        {
            res = i->second->find_items_prioritized(domain);
            if (!res.empty())
                return res.front();
        }

    } else {
        // Scan only one directory set, the one mentioned in the function parameters.
        if (directory_sets.find(defaultSet) != directory_sets.end())
        {
            res = directory_sets[defaultSet]->find_items_prioritized(domain);
            if (!res.empty())
                return res.front();
        }
    }
    // Return empty item if no result found
    return SRef<Directory_Set_Item*>();
}

SRef<Directory_Set*> Dir_Cache_Manager::get_directory_set(std::string key)
{
    if (directory_sets.find(key) != directory_sets.end())
        return directory_sets[key];
    return SRef<Directory_Set*>();
}

std::string Dir_Cache_Manager::add_directory(SRef<Directory_Set_Item*> dirItem, std::string setKey)
{
    if (0 == setKey.length())
    {
        setKey = get_new_directory_key();
    }
    if (directory_sets.find(setKey) == directory_sets.end())
    {
        directory_sets[setKey] = SRef<Directory_Set*>(new Directory_Set());
    }
    directory_sets[setKey]->add_item(dirItem);
    return setKey;
}

std::string Dir_Cache_Manager::add_directory_ldap(std::string url, std::string subTree, const std::string setKey)
{
    return add_directory(SRef<Directory_Set_Item*>(new Directory_Set_Item(url, subTree)), setKey != "" ? setKey : get_new_directory_key());
}
std::string Dir_Cache_Manager::get_new_directory_key() const
{
    std::string newName = "dirset";
    int num = 1;
    while (directory_sets.find(newName + itoa(num)) != directory_sets.end())
        num++;
    return newName;
}
