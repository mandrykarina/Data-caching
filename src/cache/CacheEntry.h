#pragma once

#include <chrono>

template <typename T>
struct CacheEntry
{
    T data;
    size_t access_count;
    std::chrono::steady_clock::time_point last_access;

    CacheEntry() : access_count(0), last_access(std::chrono::steady_clock::now()) {}
    CacheEntry(const T &d) : data(d), access_count(0), last_access(std::chrono::steady_clock::now()) {}
};
