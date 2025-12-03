#include "Benchmark.h"
#include "../cache/CacheManager.h"
#include "../data_structures/BTree.h"
#include "../data_structures/Sequence.h"
#include <iostream>
#include <random>
#include <iomanip>

using namespace std;

// -----------------------------------------
// Generate Zipf-like access pattern
// -----------------------------------------
Sequence<int> generate_zipf_pattern(int size, int requests)
{
    Sequence<int> seq;
    random_device rd;
    mt19937 gen(rd());

    for (int i = 0; i < requests; i++)
    {
        if (gen() % 100 < 80)
            seq.push_back(gen() % (size / 5 + 1));
        else
            seq.push_back((size / 5) + (gen() % (size - size / 5)));
    }
    return seq;
}

// -----------------------------------------
// Generate random access pattern
// -----------------------------------------
Sequence<int> generate_random_pattern(int size, int requests)
{
    Sequence<int> seq;
    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dis(0, size - 1);

    for (int i = 0; i < requests; i++)
        seq.push_back(dis(gen));

    return seq;
}

// -----------------------------------------
// Benchmark: cache vs direct storage
// -----------------------------------------
BenchmarkResult benchmark_compare(CacheManager<int> &cache, const Sequence<int> &data,
                                  const Sequence<int> &pattern)
{
    BenchmarkResult result;

    // -------------------------
    // Cache time
    // -------------------------
    auto t1 = chrono::high_resolution_clock::now();

    for (size_t i = 0; i < pattern.get_size(); i++)
        cache.get(pattern[i]);

    auto t2 = chrono::high_resolution_clock::now();
    double cache_ms = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();

    // -------------------------
    // Storage (BTree)
    // -------------------------
    BTree<int> tree;
    for (size_t i = 0; i < data.get_size(); i++)
        tree.insert(data[i]);

    t1 = chrono::high_resolution_clock::now();

    for (size_t i = 0; i < pattern.get_size(); i++)
        tree.search_slow(pattern[i]);

    t2 = chrono::high_resolution_clock::now();
    double storage_ms = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();

    // Fill structure
    auto stats = cache.get_statistics();

    result.cache_size = cache.get_max_cache_size();
    result.data_size = data.get_size();
    result.num_requests = pattern.get_size();
    result.time_cache_total_ms = cache_ms;
    result.time_storage_total_ms = storage_ms;
    result.hits = stats.hits;
    result.misses = stats.misses;
    result.hit_rate = stats.hit_rate;
    result.speedup = storage_ms / cache_ms;

    return result;
}

// -----------------------------------------
// Run full benchmark suite
// -----------------------------------------
void run_all_benchmarks()
{
    cout << "\n============ RUNNING BENCHMARKS ============\n";

    // Create sample data
    Sequence<int> data;
    for (int i = 0; i < 2000; i++)
        data.push_back(i);

    int cache_sizes[] = {10, 50, 100, 200};

    for (int c : cache_sizes)
    {
        cout << "\n--- Cache size = " << c << " ---\n";

        CacheManager<int> cache(c);
        cache.initialize(data);

        // Request patterns
        auto pattern_zipf = generate_zipf_pattern(data.get_size(), 5000);
        auto pattern_rand = generate_random_pattern(data.get_size(), 5000);

        // Zipf
        BenchmarkResult r1 =
            benchmark_compare(cache, data, pattern_zipf);

        cout << "Zipf:  hits=" << r1.hits
             << " misses=" << r1.misses
             << " hit-rate=" << fixed << setprecision(2) << r1.hit_rate
             << "%  speedup=" << r1.speedup << "x\n";

        // Reset cache for new test
        cache.initialize(data);

        // Random
        BenchmarkResult r2 =
            benchmark_compare(cache, data, pattern_rand);

        cout << "Random: hits=" << r2.hits
             << " misses=" << r2.misses
             << " hit-rate=" << fixed << setprecision(2) << r2.hit_rate
             << "%  speedup=" << r2.speedup << "x\n";
    }

    cout << "\n============ BENCHMARKS FINISHED ============\n";
}
