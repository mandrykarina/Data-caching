#pragma once

#include <chrono>

template <typename T>
struct CacheEntry
{
    T data;
    size_t access_count;
    std::chrono::milliseconds last_access;

    CacheEntry() : access_count(0), last_access(0) {}
    CacheEntry(const T &d) : data(d), access_count(0), last_access(0) {}
};
