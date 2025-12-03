#pragma once

#include "Sequence.h"
#include <unordered_map>

template <typename K, typename V>
struct Pair
{
    K key;
    V value;
    Pair() = default;
    Pair(const K &k, const V &v) : key(k), value(v) {}
    bool operator==(const Pair &other) const { return key == other.key; }
};

template <typename K, typename V>
class Dictionary
{
private:
    using Entry = Pair<K, V>;
    using Bucket = Sequence<Entry>;

    Bucket *buckets;
    size_t capacity;
    size_t size;
    static constexpr size_t INITIAL_CAPACITY = 16;
    static constexpr double LOAD_FACTOR_EXPAND = 0.75;

    size_t hash_code(const K &key) const
    {
        return std::hash<K>()(key);
    }

    size_t get_bucket_index(const K &key) const
    {
        return hash_code(key) % capacity;
    }

    void rehash(size_t new_capacity)
    {
        if (new_capacity == 0)
            return;

        Bucket *old_buckets = buckets;
        size_t old_capacity = capacity;

        buckets = new Bucket[new_capacity];
        capacity = new_capacity;

        for (size_t i = 0; i < old_capacity; ++i)
        {
            for (size_t j = 0; j < old_buckets[i].get_size(); ++j)
            {
                const auto &entry = old_buckets[i][j];
                size_t new_idx = get_bucket_index(entry.key);
                buckets[new_idx].push_back(entry);
            }
        }
        delete[] old_buckets;
    }

public:
    Dictionary() : buckets(nullptr), capacity(INITIAL_CAPACITY), size(0)
    {
        buckets = new Bucket[capacity];
    }

    ~Dictionary()
    {
        if (buckets)
            delete[] buckets;
    }

    void insert(const K &key, const V &value)
    {
        size_t idx = get_bucket_index(key);

        for (size_t i = 0; i < buckets[idx].get_size(); ++i)
        {
            if (buckets[idx][i].key == key)
            {
                buckets[idx][i].value = value;
                return;
            }
        }

        buckets[idx].push_back(Entry(key, value));
        size++;

        if (size >= capacity * LOAD_FACTOR_EXPAND)
        {
            rehash(capacity * 2);
        }
    }

    V *find(const K &key)
    {
        size_t idx = get_bucket_index(key);
        for (size_t i = 0; i < buckets[idx].get_size(); ++i)
        {
            if (buckets[idx][i].key == key)
            {
                return &buckets[idx][i].value;
            }
        }
        return nullptr;
    }

    const V *find(const K &key) const
    {
        size_t idx = get_bucket_index(key);
        for (size_t i = 0; i < buckets[idx].get_size(); ++i)
        {
            if (buckets[idx][i].key == key)
            {
                return &buckets[idx][i].value;
            }
        }
        return nullptr;
    }

    bool contains(const K &key) const
    {
        return find(key) != nullptr;
    }

    bool erase(const K &key)
    {
        size_t idx = get_bucket_index(key);
        for (size_t i = 0; i < buckets[idx].get_size(); ++i)
        {
            if (buckets[idx][i].key == key)
            {
                buckets[idx].erase(i);
                size--;
                return true;
            }
        }
        return false;
    }

    void clear()
    {
        for (size_t i = 0; i < capacity; ++i)
        {
            buckets[i].clear();
        }
        size = 0;
    }

    size_t get_size() const { return size; }
    size_t get_capacity() const { return capacity; }

    Sequence<Entry> get_all_entries() const
    {
        Sequence<Entry> result;
        for (size_t i = 0; i < capacity; ++i)
        {
            for (size_t j = 0; j < buckets[i].get_size(); ++j)
            {
                result.push_back(buckets[i][j]);
            }
        }
        return result;
    }
};
