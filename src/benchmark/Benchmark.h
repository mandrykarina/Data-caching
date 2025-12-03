#pragma once

#include "../cache/CacheManager.h"
#include "../cache/CacheStats.h"
#include "../data_structures/Sequence.h"
#include "../data_structures/BTree.h"
#include <chrono>
#include <random>
#include <vector>
#include <fstream>
#include <iomanip>
#include <string>

void run_all_benchmarks();

template <typename T>
class CacheBenchmark
{
private:
    using HighResClock = std::chrono::high_resolution_clock;
    std::vector<BenchmarkResult> results;

    Sequence<int> generate_zipf_access_pattern(size_t data_size, size_t num_requests)
    {
        Sequence<int> pattern;
        std::random_device rd;
        std::mt19937 gen(rd());

        for (size_t i = 0; i < num_requests; ++i)
        {
            if ((gen() % 100) < 80)
            {
                int idx = gen() % (static_cast<int>(std::max<size_t>(1, data_size / 5)));
                pattern.push_back(idx);
            }
            else
            {
                int idx = static_cast<int>(data_size / 5) + (gen() % static_cast<int>(std::max<size_t>(1, data_size - data_size / 5)));
                pattern.push_back(idx);
            }
        }
        return pattern;
    }

    Sequence<int> generate_random_access_pattern(size_t data_size, size_t num_requests)
    {
        Sequence<int> pattern;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0, static_cast<int>(data_size > 0 ? data_size - 1 : 0));

        for (size_t i = 0; i < num_requests; ++i)
        {
            pattern.push_back(dis(gen));
        }
        return pattern;
    }

public:
    CacheBenchmark() {}

    BenchmarkResult run_cache_test(
        CacheManager<T> &cache_manager,
        const std::string &test_name,
        const Sequence<T> &data,
        size_t num_requests,
        bool use_zipf = true)
    {

        cache_manager.initialize(data);

        Sequence<int> access_pattern = use_zipf
                                           ? generate_zipf_access_pattern(data.get_size(), num_requests)
                                           : generate_random_access_pattern(data.get_size(), num_requests);

        BenchmarkResult result;
        result.test_name = test_name;
        result.cache_size = cache_manager.get_max_cache_size();
        result.data_size = data.get_size();
        result.num_requests = num_requests;

        auto start = HighResClock::now();
        for (size_t i = 0; i < access_pattern.get_size(); ++i)
        {
            int key = access_pattern[i];
            cache_manager.get(key);
        }
        auto end = HighResClock::now();

        auto duration_cache = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        result.time_cache_total_ms = duration_cache.count() / 1000.0;

        CacheStats stats = cache_manager.get_statistics();
        result.hits = stats.hits;
        result.misses = stats.misses;
        result.hit_rate = stats.hit_rate;

        start = HighResClock::now();
        BTree<T> direct_storage;
        for (size_t i = 0; i < data.get_size(); ++i)
            direct_storage.insert(data[i]);

        for (size_t i = 0; i < access_pattern.get_size(); ++i)
            direct_storage.search_slow(access_pattern[i]);

        end = HighResClock::now();
        auto duration_storage = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        result.time_storage_total_ms = duration_storage.count() / 1000.0;

        result.speedup = (result.time_cache_total_ms > 0.0) ? (result.time_storage_total_ms / result.time_cache_total_ms) : 1.0;

        results.push_back(result);
        return result;
    }

    const std::vector<BenchmarkResult> &get_results() const
    {
        return results;
    }

    void save_to_csv(const std::string &filename) const
    {
        std::ofstream file(filename);
        file << "Test Name,Cache Size,Data Size,Requests,Time (Cache) ms,Time (Direct) ms,Speedup,Cache Hits,Cache Misses,Hit Rate %\n";
        for (const auto &r : results)
        {
            file << r.test_name << "," << r.cache_size << "," << r.data_size << "," << r.num_requests << ","
                 << std::fixed << std::setprecision(4) << r.time_cache_total_ms << "," << r.time_storage_total_ms << ","
                 << r.speedup << "," << r.hits << "," << r.misses << "," << r.hit_rate << "\n";
        }
    }
};
