#include <iostream>
#include <cassert>
#include <random>
#include <chrono>
#include <sstream>

#include "test_all.h"
#include "../cache/CacheManager.h"
#include "../data_structures/Sequence.h"
#include "../data_structures/BTree.h"
#include "../data_structures/Dictionary.h"

using namespace std;

static void header(const string &name)
{
    cout << "\n=============================================\n";
    cout << name << "\n";
    cout << "=============================================\n";
}

// Sequence tests
static void test_sequence_basic()
{
    header("Sequence: Basic operations");

    Sequence<int> s;
    assert(s.get_size() == 0);

    for (int i = 0; i < 50; ++i)
        s.push_back(i);
    assert(s.get_size() == 50);

    // Check indexing
    for (int i = 0; i < 50; ++i)
        assert(s[i] == i);

    // Insert middle
    s.insert(10, 999);
    assert(s[10] == 999);
    assert(s.get_size() == 51);

    // Erase
    s.erase(10);
    assert(s[10] == 10);
    assert(s.get_size() == 50);

    // pop_back
    s.pop_back();
    assert(s.get_size() == 49);

    // copy ctor
    Sequence<int> copy = s;
    assert(copy.get_size() == s.get_size());
    for (size_t i = 0; i < s.get_size(); ++i)
        assert(copy[i] == s[i]);

    cout << "Sequence basic tests: OK\n";
}

// Dictionary tests
static void test_dictionary_basic()
{
    header("Dictionary: Basic operations");

    Dictionary<int, string> d;
    assert(d.get_size() == 0);

    d.insert(1, "one");
    d.insert(2, "two");
    d.insert(3, "three");
    assert(d.get_size() == 3);

    auto p = d.find(2);
    assert(p && *p == "two");

    d.insert(2, "dos");
    p = d.find(2);
    assert(p && *p == "dos");

    bool erased = d.erase(2);
    assert(erased);
    assert(!d.contains(2));
    assert(d.get_size() == 2);

    // rehashing test — insert many keys
    for (int i = 100; i < 200; ++i)
        d.insert(i, "x");
    assert(d.get_size() >= 102);

    cout << "Dictionary basic tests: OK\n";
}

// BTree tests
static void test_btree_basic()
{
    header("BTree: Insert & Search");

    BTree<int> tree;
    const int N = 200;
    for (int i = 0; i < N; ++i)
        tree.insert(i);

    // contains
    for (int i = 0; i < N; ++i)
        assert(tree.contains(i));

    // search pointer validation
    for (int i = 0; i < N; ++i)
    {
        int *p = tree.search(i);
        assert(p != nullptr && *p == i);
    }

    cout << "BTree basic tests: OK\n";
}

// LFU Cache tests
static void test_cache_lfu_behavior()
{
    header("CacheManager (LFU): Hit/Miss & Eviction behavior");

    Sequence<int> data;
    for (int i = 0; i < 10; ++i)
        data.push_back(i);

    CacheManager<int> cache(3);
    cache.initialize(data);

    // Initially cache preloaded with keys 0..2
    assert(cache.get_cache_size() <= cache.get_max_cache_size());

    // Increase freq for keys 1 and 2
    for (int i = 0; i < 5; ++i)
    {
        cache.get(1);
        cache.get(2);
    }

    // Access a rare key 9 once
    cache.get(9);

    // Force eviction by accessing a new key
    cache.get(5);

    // Now check that one of less-frequently used keys got evicted.
    // Keys 1 and 2 should remain (they were frequently accessed).
    auto e1 = cache.get_cache_entry(1);
    auto e2 = cache.get_cache_entry(2);
    assert(e1 != nullptr);
    assert(e2 != nullptr);

    // Cache size constraint
    assert(cache.get_cache_size() <= cache.get_max_cache_size());

    cout << "Cache LFU behavior tests: OK\n";
}

// Cache statistical tests and stress
static void test_cache_stats_and_stress()
{
    header("CacheManager: Stats & Stress");

    Sequence<int> data;
    for (int i = 0; i < 500; ++i)
        data.push_back(i);

    CacheManager<int> cache(50);
    cache.initialize(data);

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> dist(0, 499);

    const int REQUESTS = 2000;
    for (int i = 0; i < REQUESTS; ++i)
    {
        int k = dist(gen);
        cache.get(k);
    }

    auto s = cache.get_statistics();
    // sanity checks:
    assert(s.total_accesses == static_cast<size_t>(REQUESTS));
    assert(s.hits + s.misses == static_cast<size_t>(REQUESTS));
    assert(s.hit_rate >= 0.0 && s.hit_rate <= 100.0);

    cout << "Cache stats & stress tests: OK\n";
}

// Benchmark smoke test
static void test_benchmark_smoke()
{
    header("Benchmark: Smoke test (runs small benchmark)");

    Sequence<int> data;
    for (int i = 0; i < 500; ++i)
        data.push_back(i);

    CacheManager<int> cache(50);
    cache.initialize(data);

    vector<int> pattern(1000);
    for (auto &x : pattern)
        x = rand() % 500;

    auto t1 = chrono::high_resolution_clock::now();
    for (int k : pattern)
        cache.get(k);
    auto t2 = chrono::high_resolution_clock::now();
    double cache_ms = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();

    BTree<int> tree;
    for (int i = 0; i < 500; ++i)
        tree.insert(i);

    t1 = chrono::high_resolution_clock::now();
    for (int k : pattern)
        tree.search_slow(k);
    t2 = chrono::high_resolution_clock::now();
    double store_ms = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();

    // We don't require store_ms > cache_ms always — but both should be non-negative
    assert(cache_ms >= 0.0 && store_ms >= 0.0);

    cout << "Benchmark smoke test: OK\n";
}

void run_all_tests()
{
    cout << "\n==== RUNNING FULL TEST SUITE ====\n";
    test_sequence_basic();
    test_dictionary_basic();
    test_btree_basic();
    test_cache_lfu_behavior();
    test_cache_stats_and_stress();
    test_benchmark_smoke();
    cout << "\n===== ALL TESTS PASSED SUCCESSFULLY =====\n";
}
