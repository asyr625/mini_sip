#ifndef DIRECTORY_SET_ITEM_H
#define DIRECTORY_SET_ITEM_H

#include "my_types.h"
#include "cache_item.h"

#define DIRECTORYSETITEM_TYPE_UNSPECIFIED 0
#define DIRECTORYSETITEM_TYPE_LDAP 1

class Directory_Set_Item : public Cache_Item
{
public:
    Directory_Set_Item();
    Directory_Set_Item(std::string url, std::string subTree);
    Directory_Set_Item(int32_t type, std::string url, std::string subTree);
    Directory_Set_Item(int32_t type);

    int32_t get_ype() const;
    std::string get_url() const;
    std::string get_sub_tree() const;

    void set_type(const int32_t type);
    void set_url(const std::string url);
    void set_sub_tree(const std::string subTree);

    virtual std::string get_mem_object_type() const { return "DirectorySetItem"; }

    bool operator ==(const Directory_Set_Item item2) const;
private:
    int32_t type;
    std::string url;
    std::string sub_tree;
};

#endif // DIRECTORY_SET_ITEM_H
