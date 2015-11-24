#include "directory_set.h"

#include "string_utils.h"

#include <algorithm>

Directory_Set::Directory_Set()
{
    _items_index = _items.begin();
}

Directory_Set::~Directory_Set()
{
    std::list<SRef<Directory_Set_Item*> >::iterator i;
    std::list<SRef<Directory_Set_Item*> >::iterator last = _items.end();
    for( i = _items.begin(); i != last; i++ )
        _items.clear();
}

Directory_Set * Directory_Set::create()
{
    return new Directory_Set();
}

Directory_Set* Directory_Set::clone()
{
    Directory_Set * db = create();

    lock();
    std::list<SRef<Directory_Set_Item*> >::iterator i;
    std::list<SRef<Directory_Set_Item*> >::iterator last = _items.end();

    for( i = _items.begin(); i != last; i++ )
    {
        db->add_item( *i );
    }

    unlock();
    return db;
}


void Directory_Set::lock()
{
    _mlock.lock();
}

void Directory_Set::unlock()
{
    _mlock.unlock();
}

void Directory_Set::add_item (const SRef<Directory_Set_Item*> item)
{
    _items.push_back( item );
    _items_index = _items.begin();
}

SRef<Directory_Set_Item*> Directory_Set::create_item_ldap(const std::string url, const std::string subTree)
{
    SRef<Directory_Set_Item*> item = new Directory_Set_Item(url, subTree);

    return item;
}

void Directory_Set::add_ldap (const std::string url, const std::string subTree)
{
    SRef<Directory_Set_Item*> item = create_item_ldap(url, subTree);
    add_item(item);
}

void Directory_Set::remove (SRef<Directory_Set_Item*> removedItem)
{
    init_index();

    while( _items_index != _items.end() ){
        if( **(*_items_index) == **removedItem ){
            _items.erase( _items_index );
            init_index();
            return;
        }
        _items_index ++;
    }
    init_index();
}

std::list<SRef<Directory_Set_Item*> > & Directory_Set::get_items()
{
    return _items;
}

std::list<SRef<Directory_Set_Item*> > Directory_Set::find_items(const std::string subTree, const bool endsWithIsEnough)
{
    std::list<SRef<Directory_Set_Item*> > res;
    init_index();

    while( _items_index != _items.end() ){
        SRef<Directory_Set_Item*> item = *_items_index;

        if ((endsWithIsEnough && string_ends_with(item->get_sub_tree(), subTree)) || item->get_sub_tree() == subTree)
            res.push_back(item);

        _items_index++;
    }
    init_index();
    return res;
}

bool directorySetItemUrlComparator(SRef<Directory_Set_Item*> a, SRef<Directory_Set_Item*> b)
{
    return (a->get_sub_tree().length() > b->get_sub_tree().length());
}

std::vector<SRef<Directory_Set_Item*> > Directory_Set::find_items_prioritized(const std::string domain)
{
    std::vector<SRef<Directory_Set_Item*> > res;
    /*
    PHASE 1: Picking out candidate directory server.
    */
    init_index();

    while( _items_index != _items.end() ){
        SRef<Directory_Set_Item*> item = *_items_index;
        if (string_ends_with(domain, item->get_sub_tree()))
            res.push_back(item);

        _items_index++;
    }

    /*
    PHASE 2: Sorting the candidates. Longest URL first.
    */
    std::sort(res.begin(), res.end(), directorySetItemUrlComparator);

    /*
    CONCLUDE =)
    */
    init_index();
    return res;
}

SRef<Directory_Set_Item*> Directory_Set::get_next()
{
    SRef<Directory_Set_Item*> tmp;

    if( _items_index == _items.end() ){
        _items_index = _items.begin();
        return NULL;
    }

    tmp = *_items_index;
    _items_index++;
    return tmp;
}

void Directory_Set::init_index()
{
    _items_index = _items.begin();
}
