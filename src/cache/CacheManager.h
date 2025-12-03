#pragma once

#include <stdexcept>
#include "../data_structures/Dictionary.h"
#include "../data_structures/BTree.h"
#include "../data_structures/Sequence.h"
#include "CacheEntry.h"
#include "CacheStats.h"
#include <chrono>

template <typename T>
class CacheManager
{
private:
    Dictionary<int, CacheEntry<T>> cache;
    BTree<T> storage;
    Sequence<T> all_data;
    size_t cache_size;
    CacheStats stats;

    int find_lfu_key() const
    {
        auto entries = cache.get_all_entries();
        if (entries.get_size() == 0)
            throw std::runtime_error("Cache is empty");

        // use the first entry as initial minimum
        int lfu_key = entries[0].key;
        size_t min_count = entries[0].value.access_count;

        for (size_t i = 1; i < entries.get_size(); ++i)
        {
            if (entries[i].value.access_count < min_count)
            {
                min_count = entries[i].value.access_count;
                lfu_key = entries[i].key;
            }
        }
        return lfu_key;
    }

    void evict()
    {
        if (cache.get_size() > 0)
        {
            int lfu_key = find_lfu_key();
            cache.erase(lfu_key);
            stats.evictions++;
        }
    }

public:
    CacheManager(size_t max_cache_size) : cache_size(max_cache_size)
    {
        if (max_cache_size == 0)
            throw std::invalid_argument("Cache size must be > 0");
    }

    void initialize(const Sequence<T> &data)
    {
        all_data = data;
        storage.clear();

        for (size_t i = 0; i < data.get_size(); ++i)
        {
            storage.insert(data[i]);
        }

        cache.clear();
        stats = CacheStats();
    }

    T *get(int key)
    {
        stats.total_accesses++;

        auto entry = cache.find(key);
        if (entry)
        {
            entry->access_count++;
            stats.hits++;
            return &entry->data;
        }

        stats.misses++;
        for (size_t i = 0; i < all_data.get_size(); ++i)
        {
            if (all_data[i] == T(key))
            {
                if (cache.get_size() >= cache_size)
                {
                    evict();
                }

                CacheEntry<T> new_entry(all_data[i]);
                new_entry.access_count = 1;
                cache.insert(key, new_entry);

                auto cached = cache.find(key);
                return &cached->data;
            }
        }

        return nullptr;
    }

    CacheStats get_statistics() const
    {
        CacheStats result = stats;
        result.hit_rate = result.total_accesses > 0
                              ? (100.0 * result.hits) / result.total_accesses
                              : 0.0;
        result.avg_access_time_cache = 1.0;
        result.avg_access_time_storage = 75.0;
        result.speedup = result.avg_access_time_storage / result.avg_access_time_cache;
        return result;
    }

    const CacheEntry<T> *get_cache_entry(int key) const
    {
        return cache.find(key);
    }

    size_t get_cache_size() const { return cache.get_size(); }
    size_t get_max_cache_size() const { return cache_size; }
    size_t get_storage_size() const { return storage.get_size(); }

    void clear()
    {
        cache.clear();
        storage.clear();
        all_data.clear();
        stats = CacheStats();
    }
};
