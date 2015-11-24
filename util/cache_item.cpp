#include "cache_item.h"

Cache_Item::Cache_Item()
{
    _cache_date = time(NULL);
    if( -1 == _cache_date )
        _cache_date = 0;
    _valid_from = 0;
    _valid_until = 0;
}

time_t Cache_Item::get_cache_date() const
{
    return _cache_date;
}
time_t Cache_Item::get_valid_from() const
{
    return _valid_from;
}
time_t Cache_Item::get_valid_until() const
{
    return _valid_until;
}

void Cache_Item::set_cache_date(const time_t date)
{
    _cache_date = date;
}

void Cache_Item::set_valid_from(const time_t date)
{
    _valid_from = date;
}

void Cache_Item::set_valid_until(const time_t date)
{
    _valid_until = date;
}
