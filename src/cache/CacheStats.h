#pragma once

#include <string>
#include <vector>
#include <iomanip>

struct CacheStats
{
    size_t hits;
    size_t misses;
    size_t total_accesses;
    size_t evictions;
    double hit_rate;
    double avg_access_time_cache;
    double avg_access_time_storage;
    double speedup;

    CacheStats() : hits(0), misses(0), total_accesses(0), evictions(0),
                   hit_rate(0.0), avg_access_time_cache(0.0),
                   avg_access_time_storage(0.0), speedup(0.0) {}
};

struct BenchmarkResult
{
    std::string test_name;
    size_t cache_size;
    size_t data_size;
    size_t num_requests;
    double time_cache_total_ms;
    double time_storage_total_ms;
    double speedup;
    size_t hits;
    size_t misses;
    double hit_rate;

    BenchmarkResult() : cache_size(0), data_size(0), num_requests(0),
                        time_cache_total_ms(0.0), time_storage_total_ms(0.0),
                        speedup(0.0), hits(0), misses(0), hit_rate(0.0) {}

    std::string to_string() const
    {
        std::string result = "Test: " + test_name + "\n";
        result += " Cache Size: " + std::to_string(cache_size) + "\n";
        result += " Data Size: " + std::to_string(data_size) + "\n";
        result += " Requests: " + std::to_string(num_requests) + "\n";
        result += " Time (with cache): " + std::to_string(time_cache_total_ms) + " ms\n";
        result += " Time (direct): " + std::to_string(time_storage_total_ms) + " ms\n";
        result += " Speedup: " + std::to_string(speedup) + "x\n";
        result += " Hit Rate: " + std::to_string(hit_rate) + "%\n";
        return result;
    }
};
