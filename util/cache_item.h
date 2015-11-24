#ifndef CACHE_ITEM_H
#define CACHE_ITEM_H

#include <time.h>

#include "sobject.h"

class Cache_Item : public SObject
{
public:
    Cache_Item();

    time_t get_cache_date() const;
    time_t get_valid_from() const;
    time_t get_valid_until() const;

    void set_cache_date(const time_t date);
    void set_valid_from(const time_t date);
    void set_valid_until(const time_t date);

    virtual std::string get_mem_object_type() const { return "CacheItem"; }
private:
    time_t _cache_date;
    time_t _valid_from;
    time_t _valid_until;
};

#endif // CACHE_ITEM_H
