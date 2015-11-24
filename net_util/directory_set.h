#ifndef DIRECTORY_SET_H
#define DIRECTORY_SET_H
#include <list>
#include <vector>

#include "mutex.h"
#include "sobject.h"
#include "directory_set_item.h"

class Directory_Set : public SObject
{
public:
    Directory_Set();

    ~Directory_Set();
    static Directory_Set *create();

    Directory_Set* clone();
    void add_ldap (const std::string url, const std::string subTree);

    std::list<SRef<Directory_Set_Item*> > & get_items();

    std::list<SRef<Directory_Set_Item*> > find_items(const std::string subTree, const bool endsWithIsEnough = true);

    std::vector<SRef<Directory_Set_Item*> > find_items_prioritized(const std::string domain);

    SRef<Directory_Set_Item*> get_next();

    void init_index();
    void lock();
    void unlock();

    void remove(SRef<Directory_Set_Item*> removedItem);

    void add_item(const SRef<Directory_Set_Item*> item);

    SRef<Directory_Set_Item*> create_item_ldap(const std::string url, const std::string subTree);

private:
    std::list<SRef<Directory_Set_Item*> >::iterator _items_index;
    std::list<SRef<Directory_Set_Item*> > _items;
    Mutex _mlock;
};

#endif // DIRECTORY_SET_H
