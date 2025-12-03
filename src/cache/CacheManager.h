#pragma once

#include <stdexcept>
#include <unordered_map>
#include <list>
#include "../data_structures/BTree.h"
#include "../data_structures/Sequence.h"
#include "CacheEntry.h"
#include "CacheStats.h"
#include <chrono>
#include <algorithm>

template <typename T>
class CacheManager
{
private:
    size_t max_cache_size;

    // core cache storage: key -> CacheEntry
    std::unordered_map<int, CacheEntry<T>> cache_map;

    // frequency lists: freq -> list of keys with that freq (front: most-recent-with-this-freq)
    std::unordered_map<size_t, std::list<int>> freq_lists;

    // map key -> iterator in corresponding list (for O(1) removal)
    std::unordered_map<int, typename std::list<int>::iterator> key_iter_map;

    // map key -> current frequency
    std::unordered_map<int, size_t> key_freq_map;

    // minimal frequency currently in cache (to quickly evict LFU)
    size_t min_freq;

    // underlying "slow" storage
    BTree<T> storage;
    Sequence<T> all_data;

    // statistics
    CacheStats stats;

    // helper: touch key -> increase its frequency
    void touch(int key)
    {
        auto itf = key_freq_map.find(key);
        if (itf == key_freq_map.end())
            return; // shouldn't happen

        size_t freq = itf->second;
        // remove from old freq list
        auto &old_list = freq_lists[freq];
        auto it_key_it = key_iter_map.find(key);
        if (it_key_it != key_iter_map.end())
            old_list.erase(it_key_it->second);

        // if old list is empty and freq == min_freq, increment min_freq
        if (old_list.empty() && freq == min_freq)
            min_freq++;

        // insert into new list with freq+1 at front (recently used)
        size_t newf = freq + 1;
        freq_lists[newf].push_front(key);
        key_iter_map[key] = freq_lists[newf].begin();
        key_freq_map[key] = newf;
    }

    void evict_one()
    {
        if (cache_map.empty())
            return;
        // find list for min_freq
        auto it = freq_lists.find(min_freq);
        if (it == freq_lists.end() || it->second.empty())
        {
            // find next non-empty freq
            for (auto &p : freq_lists)
            {
                if (!p.second.empty())
                {
                    min_freq = p.first;
                    break;
                }
            }
            it = freq_lists.find(min_freq);
            if (it == freq_lists.end() || it->second.empty())
                return;
        }

        // Evict the least recently used among those with min_freq -> take back()
        int victim_key = it->second.back();
        it->second.pop_back();

        // erase maps
        key_iter_map.erase(victim_key);
        key_freq_map.erase(victim_key);
        cache_map.erase(victim_key);

        stats.evictions++;
        // if the list became empty, erase it
        if (it->second.empty())
            freq_lists.erase(it);
    }

public:
    CacheManager(size_t capacity = 100) : max_cache_size(capacity), min_freq(0)
    {
        if (capacity == 0)
            throw std::invalid_argument("Cache capacity must be > 0");
        stats = CacheStats();
    }

    void initialize(const Sequence<T> &data)
    {
        // prepare slow storage
        all_data = data;
        storage.clear();
        for (size_t i = 0; i < data.get_size(); ++i)
            storage.insert(data[i]);

        // clear cache structures
        cache_map.clear();
        freq_lists.clear();
        key_iter_map.clear();
        key_freq_map.clear();
        min_freq = 0;
        stats = CacheStats();

        // Preload cache with first min(max_cache_size, data_size) items
        size_t preload = std::min(max_cache_size, data.get_size());
        for (size_t i = 0; i < preload; ++i)
        {
            int key = static_cast<int>(i);
            CacheEntry<T> e(all_data[i]);
            e.access_count = 1;
            e.last_access = std::chrono::steady_clock::now();
            cache_map[key] = e;
            key_freq_map[key] = 1;
            freq_lists[1].push_front(key);
            key_iter_map[key] = freq_lists[1].begin();
        }
        if (preload > 0)
            min_freq = 1;
    }

    // get returns pointer to data in cache (or loads it)
    T *get(int key)
    {
        stats.total_accesses++;
        auto start = std::chrono::steady_clock::now();

        // If found in cache
        auto it = cache_map.find(key);
        if (it != cache_map.end())
        {
            // update per-LFU structures
            it->second.access_count++;
            it->second.last_access = std::chrono::steady_clock::now();
            stats.hits++;
            touch(key);

            auto end = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;
            // update avg access time (simple moving average)
            stats.avg_access_time_cache = (stats.avg_access_time_cache * (stats.hits + stats.misses - 1) + elapsed) / (stats.hits + stats.misses);
            return &it->second.data;
        }

        // Miss: try to get from all_data by index (fast path)
        stats.misses++;
        T *value_ptr = nullptr;
        if (key >= 0 && static_cast<size_t>(key) < all_data.get_size())
        {
            value_ptr = &all_data[static_cast<size_t>(key)];
        }
        else
        {
            // else try BTree search (maybe T can be constructed from key)
            value_ptr = storage.search(T(key));
        }

        if (!value_ptr)
            return nullptr;

        // Insert into cache, evicting if needed
        if (cache_map.size() >= max_cache_size)
        {
            evict_one();
        }

        CacheEntry<T> e(*value_ptr);
        e.access_count = 1;
        e.last_access = std::chrono::steady_clock::now();
        cache_map[key] = e;
        key_freq_map[key] = 1;
        freq_lists[1].push_front(key);
        key_iter_map[key] = freq_lists[1].begin();
        min_freq = 1;

        auto cached_it = cache_map.find(key);

        auto end = std::chrono::steady_clock::now();
        double elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count() / 1000.0;
        stats.avg_access_time_cache = (stats.avg_access_time_cache * (stats.hits + stats.misses - 1) + elapsed) / (stats.hits + stats.misses);

        return &cached_it->second.data;
    }

    // Inspectors
    CacheStats get_statistics() const
    {
        CacheStats s = stats;
        s.hit_rate = s.total_accesses > 0 ? (100.0 * s.hits) / s.total_accesses : 0.0;
        // storage avg is unknown; keep default
        s.speedup = (s.avg_access_time_cache > 0.0) ? (s.avg_access_time_storage / s.avg_access_time_cache) : 1.0;
        return s;
    }

    // Return pointer to cache entry if present (const)
    const CacheEntry<T> *get_cache_entry(int key) const
    {
        auto it = cache_map.find(key);
        if (it == cache_map.end())
            return nullptr;
        return &it->second;
    }

    size_t get_cache_size() const { return cache_map.size(); }
    size_t get_max_cache_size() const { return max_cache_size; }
    size_t get_storage_size() const { return storage.get_size(); }

    // Expose cache content for inspection: returns copy of key list (unordered)
    Sequence<int> get_cache_keys() const
    {
        Sequence<int> keys;
        for (const auto &p : cache_map)
            keys.push_back(p.first);
        return keys;
    }

    void clear()
    {
        cache_map.clear();
        freq_lists.clear();
        key_iter_map.clear();
        key_freq_map.clear();
        min_freq = 0;
        storage.clear();
        all_data.clear();
        stats = CacheStats();
    }
};
