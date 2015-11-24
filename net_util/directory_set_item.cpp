#include "directory_set_item.h"

Directory_Set_Item::Directory_Set_Item() :
    type(DIRECTORYSETITEM_TYPE_LDAP), url(""), sub_tree("")
{}

Directory_Set_Item::Directory_Set_Item(std::string url, std::string subTree)
    : type(DIRECTORYSETITEM_TYPE_LDAP), url(url), sub_tree(subTree)
{}

Directory_Set_Item::Directory_Set_Item(int32_t type, std::string url, std::string subTree)
    : type(type), url(url), sub_tree(subTree)
{}

Directory_Set_Item::Directory_Set_Item(int32_t type)
    : type(type), url(""), sub_tree("")
{}

bool Directory_Set_Item::operator == (const Directory_Set_Item item2) const
{
    return ( item2.get_url() == url && item2.get_ype() == type && item2.get_sub_tree() == sub_tree );
}

int32_t Directory_Set_Item::get_ype() const
{
    return type;
}

std::string Directory_Set_Item::get_url() const
{
    return url;
}

std::string Directory_Set_Item::get_sub_tree() const
{
    return sub_tree;
}

void Directory_Set_Item::set_type(const int32_t type)
{
    this->type = type;
}

void Directory_Set_Item::set_url(const std::string url)
{
    this->url = url;
}

void Directory_Set_Item::set_sub_tree(const std::string subTree)
{
    this->sub_tree = subTree;
}
