#include <iostream>
#include <cassert>
#include <random>
#include <chrono>

#include "test_all.h"
#include "../cache/CacheManager.h"
#include "../data_structures/Sequence.h"
#include "../data_structures/BTree.h"

using namespace std;

static void header(const string &name)
{
    cout << "\n=============================================\n";
    cout << name << "\n";
    cout << "=============================================\n";
}

//
// TEST 1 — Initialization
//
static void test_initialization()
{
    header("TEST 1: Initialization");

    Sequence<int> data;
    for (int i = 0; i < 100; i++)
        data.push_back(i);

    CacheManager<int> cache(10);
    cache.initialize(data);

    assert(cache.get_cache_size() == 0);
    assert(cache.get_storage_size() == 100);

    cout << "OK\n";
}

//
// TEST 2 — Hits / Misses
//
static void test_hits_misses()
{
    header("TEST 2: Hits & Misses");

    Sequence<int> data;
    for (int i = 0; i < 50; i++)
        data.push_back(i);

    CacheManager<int> cache(5);
    cache.initialize(data);

    cache.get(10); // miss
    cache.get(10); // hit

    auto s = cache.get_statistics();
    assert(s.total_accesses == 2);
    assert(s.hits == 1);
    assert(s.misses == 1);

    cout << "OK\n";
}

//
// TEST 3 — LFU eviction
//
static void test_lfu()
{
    header("TEST 3: LFU eviction");

    Sequence<int> data;
    for (int i = 0; i < 10; i++)
        data.push_back(i);

    CacheManager<int> cache(3);
    cache.initialize(data);

    for (int i = 0; i < 5; i++)
    {
        cache.get(1);
        cache.get(2);
    }

    cache.get(9); // rare
    cache.get(5); // forces eviction

    auto e1 = cache.get_cache_entry(1);
    auto e2 = cache.get_cache_entry(2);
    auto e9 = cache.get_cache_entry(9);

    assert(e1 != nullptr);
    assert(e2 != nullptr);
    assert(e9 == nullptr); // evicted

    cout << "OK\n";
}

//
// TEST 4 — 1000 random requests
//
static void test_random()
{
    header("TEST 4: Random load (1000 requests)");

    Sequence<int> data;
    for (int i = 0; i < 200; i++)
        data.push_back(i);

    CacheManager<int> cache(20);
    cache.initialize(data);

    random_device rd;
    mt19937 gen(rd());
    uniform_int_distribution<> d(0, 199);

    for (int i = 0; i < 1000; i++)
        cache.get(d(gen));

    auto s = cache.get_statistics();
    assert(s.total_accesses == 1000);

    cout << "OK\n";
}

//
// TEST 5 — Speed comparison Cache vs BTree
//
static void test_speed()
{
    header("TEST 5: Cache vs BTree speed test");

    Sequence<int> data;
    for (int i = 0; i < 500; i++)
        data.push_back(i);

    CacheManager<int> cache(50);
    cache.initialize(data);

    vector<int> pattern(5000);
    for (auto &x : pattern)
        x = rand() % 500;

    // cache time
    auto t1 = chrono::high_resolution_clock::now();
    for (int x : pattern)
        cache.get(x);
    auto t2 = chrono::high_resolution_clock::now();
    double cache_ms = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();

    // storage time
    BTree<int> tree;
    for (int i = 0; i < 500; i++)
        tree.insert(i);

    t1 = chrono::high_resolution_clock::now();
    for (int x : pattern)
        tree.search_slow(x);
    t2 = chrono::high_resolution_clock::now();
    double store_ms = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count();

    assert(store_ms > cache_ms);

    cout << "OK\n";
}

//
// RUN ALL
//
void run_all_tests()
{
    test_initialization();
    test_hits_misses();
    test_lfu();
    test_random();
    test_speed();

    cout << "\n===== ALL TESTS PASSED SUCCESSFULLY =====\n";
}
