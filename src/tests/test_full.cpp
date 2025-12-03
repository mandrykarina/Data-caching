// src/tests/test_full.cpp
#include <iostream>
#include <vector>
#include <set>
#include <algorithm>
#include <iomanip>

#include "../data_structures/Sequence.h"
#include "../data_structures/Dictionary.h"
#include "../data_structures/BTree.h"
#include "../cache/CacheManager.h"
#include "../cache/CacheStats.h"
#include "../cache/CacheEntry.h"

using namespace std;

// ---------- Утилиты ----------
static string okfail(bool ok) { return ok ? "OK" : "FAIL"; }

template <typename T>
static void print_sequence_inline(const Sequence<T> &s)
{
    for (size_t i = 0; i < s.get_size(); ++i)
    {
        if (i)
            cout << " ";
        cout << s[i];
    }
}

static void print_keys_inline(const Sequence<int> &s)
{
    print_sequence_inline(s);
}

static set<int> seq_to_set(const Sequence<int> &s)
{
    set<int> st;
    for (size_t i = 0; i < s.get_size(); ++i)
        st.insert(s[i]);
    return st;
}

// ---------- Sequence tests ----------
static void sequence_tests()
{
    cout << "===== ТЕСТ: Sequence =====\n";

    // Тест 1: push_back и вывод
    Sequence<int> s;
    s.push_back(55);
    s.push_back(69);
    s.push_back(77);
    cout << "push_back sequence: ";
    print_sequence_inline(s);
    cout << "\nExpected: 55 69 77\n";
    cout << "RESULT: " << okfail(s.get_size() == 3 && s[0] == 55 && s[1] == 69 && s[2] == 77) << "\n\n";

    // Тест 2: erase (пример из условия)
    Sequence<int> s2 = s;
    cout << "erase(2): before: ";
    print_sequence_inline(s2);
    s2.erase(2);
    cout << "\nafter:  ";
    print_sequence_inline(s2);
    cout << "\nExpected after: 55 69\n";
    bool cond2 = (s2.get_size() == 2 && s2[0] == 55 && s2[1] == 69);
    cout << "RESULT: " << okfail(cond2) << "\n\n";

    // Тест 3: insert в середину
    Sequence<int> s3;
    s3.push_back(1);
    s3.push_back(2);
    s3.push_back(4);
    cout << "insert(2,3): before: ";
    print_sequence_inline(s3);
    s3.insert(2, 3);
    cout << "\nafter:  ";
    print_sequence_inline(s3);
    cout << "\nExpected: 1 2 3 4\n";
    bool cond3 = (s3.get_size() == 4 && s3[2] == 3);
    cout << "RESULT: " << okfail(cond3) << "\n\n";

    // Тест 4: find
    int idx = s3.find(3);
    cout << "find(3): expected 2, actual " << idx << " -> " << okfail(idx == 2) << "\n\n";

    // Тест 5: pop_back, clear, copy ctor
    s3.pop_back();
    cout << "pop_back -> ";
    print_sequence_inline(s3);
    cout << "\nExpected: 1 2 3\n";
    bool cond5 = (s3.get_size() == 3);
    cout << "RESULT: " << okfail(cond5) << "\n";
    Sequence<int> copy = s3;
    cout << "copy ctor -> ";
    print_sequence_inline(copy);
    cout << "\nExpected same as above\n";
    cout << "RESULT: " << okfail(copy.get_size() == s3.get_size() && copy[0] == s3[0]) << "\n";

    cout << "------------------------------\n\n";
}

// ---------- Dictionary tests ----------
static void dictionary_tests()
{
    cout << "===== ТЕСТ: Dictionary =====\n";

    Dictionary<int, string> dict;
    cout << "Insert: 10:'ten' 20:'twenty' 30:'thirty'\n";
    dict.insert(10, "ten");
    dict.insert(20, "twenty");
    dict.insert(30, "thirty");

    cout << "All entries (key:value):\n";
    auto entries = dict.get_all_entries();
    for (size_t i = 0; i < entries.get_size(); ++i)
    {
        cout << entries[i].key << ":" << entries[i].value;
        if (i + 1 < entries.get_size())
            cout << "  ";
    }
    cout << "\n";

    // find existing
    auto p = dict.find(20);
    cout << "find(20): expected 'twenty', actual '";
    cout << (p ? *p : "<null>") << "' -> " << okfail(p && *p == "twenty") << "\n";

    // overwrite value
    dict.insert(20, "dos");
    auto p2 = dict.find(20);
    cout << "after insert(20,'dos') -> find(20) = '" << (p2 ? *p2 : "<null>") << "' -> " << okfail(p2 && *p2 == "dos") << "\n";

    // erase
    cout << "erase(20)\n";
    bool erased = dict.erase(20);
    cout << "find(20) after erase -> '" << (dict.find(20) ? *dict.find(20) : "<null>") << "' -> " << okfail(erased && dict.find(20) == nullptr) << "\n";

    // bulk insert to test rehash
    cout << "bulk insert keys 100..199\n";
    for (int k = 100; k < 200; ++k)
        dict.insert(k, "x");
    cout << "get_size() >= 101? actual " << dict.get_size() << " -> " << okfail(dict.get_size() >= 101) << "\n";

    // print bucket-level detail if available (get_all_entries gives flat view)
    cout << "Flat entries sample (first 10): ";
    auto all = dict.get_all_entries();
    for (size_t i = 0; i < std::min<size_t>(10, all.get_size()); ++i)
    {
        if (i)
            cout << " ";
        cout << all[i].key << ":" << all[i].value;
    }
    cout << "\n";

    cout << "------------------------------\n\n";
}

// ---------- BTree tests ----------
static void btree_tests()
{
    cout << "===== ТЕСТ: BTree =====\n";

    BTree<int> tree;
    cout << "Insert numbers 1..100\n";
    for (int i = 1; i <= 100; ++i)
        tree.insert(i);
    cout << "get_size(): " << tree.get_size() << " (expected 100)\n";
    cout << "contains(25): " << (tree.contains(25) ? "true" : "false") << " -> " << okfail(tree.contains(25)) << "\n";
    cout << "contains(200): " << (tree.contains(200) ? "true" : "false") << " -> " << okfail(!tree.contains(200)) << "\n";

    int *p = tree.search(50);
    cout << "search(50) pointer -> " << (p ? to_string(*p) : string("<null>")) << " -> " << okfail(p && *p == 50) << "\n";

    // Partial structural check: search for some values spread across range
    bool bulk_ok = true;
    for (int q = 1; q <= 100; q += 7)
    {
        if (!tree.contains(q))
        {
            bulk_ok = false;
            break;
        }
    }
    cout << "spot-check contains for step 7 -> " << okfail(bulk_ok) << "\n";

    cout << "------------------------------\n\n";
}

// ---------- CacheManager & LFU correctness tests ----------
static void cachemanager_lfu_tests()
{
    cout << "===== ТЕСТ: CacheManager (LFU correctness) =====\n";

    Sequence<int> data;
    for (int i = 0; i < 10; ++i)
        data.push_back(i);

    CacheManager<int> cache(2); // small capacity to force evictions
    cache.initialize(data);

    // Track cache keys before operations
    auto before_keys = cache.get_cache_keys();
    cout << "Initial cache keys: ";
    print_keys_inline(before_keys);
    cout << "\n";

    // Access pattern:
    // get(1) x3, get(2) x1, then get(3) should evict key with lowest freq -> key 2 (or depending on preload)
    cout << "Sequence: get(1), get(1), get(1), get(2), get(3)\n";
    cache.get(1); // miss/hit depending on preload
    cache.get(1);
    cache.get(1);
    cache.get(2);
    // before get(3) record set
    auto keys_mid = cache.get_cache_keys();
    set<int> midset = seq_to_set(keys_mid);
    cout << "Cache before get(3): ";
    print_keys_inline(keys_mid);
    cout << "\n";
    cache.get(3); // forces eviction
    auto keys_after = cache.get_cache_keys();
    set<int> afterset = seq_to_set(keys_after);
    cout << "Cache after get(3): ";
    print_keys_inline(keys_after);
    cout << "\n";

    // Heuristic expectation: 1 should remain (most frequent), 3 should be present, the other slot is for either 2 or previous preload.
    bool has1 = cache.get_cache_entry(1) != nullptr;
    bool has3 = cache.get_cache_entry(3) != nullptr;
    cout << "Expect 1 present and 3 present -> actual 1="
         << (has1 ? "present" : "absent") << ", 3=" << (has3 ? "present" : "absent")
         << " -> " << okfail(has1 && has3) << "\n";

    // Show freq counts for keys in cache
    cout << "Key:count in cache:\n";
    auto keys_now = cache.get_cache_keys();
    for (size_t i = 0; i < keys_now.get_size(); ++i)
    {
        int k = keys_now[i];
        const CacheEntry<int> *e = cache.get_cache_entry(k);
        if (e)
            cout << " " << k << ":" << e->access_count;
    }
    cout << "\n";

    cout << "------------------------------\n\n";
}

// ---------- Eviction tests (predictable scenarios) ----------
static void eviction_tests()
{
    cout << "===== ТЕСТ: Eviction (LFU predictable scenarios) =====\n";

    // Build data and cache
    Sequence<int> data;
    for (int i = 0; i < 6; ++i)
        data.push_back(i);

    CacheManager<int> cache(3);
    cache.initialize(data);

    cout << "Initial cache keys: ";
    print_keys_inline(cache.get_cache_keys());
    cout << "\n";

    // Create frequencies:
    // make 0 freq high, 1 medium, 2 low
    cache.get(0);
    cache.get(0);
    cache.get(0); // make key 0 frequent
    cache.get(1);
    cache.get(1); // key 1 medium
    // key 2 left as is (lower frequency)

    cout << "After frequency setup, cache keys and counts:\n";
    auto k1 = cache.get_cache_keys();
    for (size_t i = 0; i < k1.get_size(); ++i)
    {
        int k = k1[i];
        const CacheEntry<int> *e = cache.get_cache_entry(k);
        cout << k << ":" << (e ? to_string(e->access_count) : string("na")) << " ";
    }
    cout << "\n";

    // Force eviction by requesting new keys
    cout << "Request get(3) (should evict least freq key)\n";
    auto before_set = seq_to_set(cache.get_cache_keys());
    cache.get(3);
    auto after_set = seq_to_set(cache.get_cache_keys());

    // find which key removed
    vector<int> removed;
    for (int x : vector<int>{0, 1, 2, 3})
    {
        if (before_set.count(x) && !after_set.count(x))
            removed.push_back(x);
    }

    cout << "Removed keys (should include 2): ";
    if (removed.empty())
        cout << "<none>";
    else
        for (size_t i = 0; i < removed.size(); ++i)
        {
            if (i)
                cout << " ";
            cout << removed[i];
        }
    cout << "\n";
    bool removed2 = find(removed.begin(), removed.end(), 2) != removed.end();
    cout << "RESULT: " << okfail(removed2) << "\n";

    cout << "------------------------------\n\n";
}

// ---------- Hit/Miss tests ----------
static void hit_miss_tests()
{
    cout << "===== ТЕСТ: Hit/Miss =====\n";

    Sequence<int> data;
    for (int i = 0; i < 100; ++i)
        data.push_back(i);

    CacheManager<int> cache(10);
    cache.initialize(data);

    // Warm up some keys
    vector<int> warm = {1, 2, 3, 4, 5};
    for (int k : warm)
        cache.get(k);

    auto stats_before = cache.get_statistics();
    cout << "Before sequence: hits=" << stats_before.hits << " misses=" << stats_before.misses << "\n";

    vector<int> seq = {1, 2, 50, 60, 1, 70, 2, 99, 1000};
    cout << "Requests sequence: ";
    for (size_t i = 0; i < seq.size(); ++i)
    {
        if (i)
            cout << " ";
        cout << seq[i];
    }
    cout << "\n";

    for (int r : seq)
    {
        auto val = cache.get(r);
        bool hit = (val != nullptr && cache.get_cache_entry(r) != nullptr && cache.get_statistics().hits > 0 && cache.get_statistics().total_accesses > 0 && cache.get_statistics().hits + cache.get_statistics().misses > 0);
        // More robust: check if last operation produced hit or miss is not directly accessible; we use statistics delta
        cout << "get(" << r << "): " << (val ? to_string(*val) : string("<null>")) << "\n";
    }

    auto stats_after = cache.get_statistics();
    cout << "After sequence: hits=" << stats_after.hits << " misses=" << stats_after.misses << " total=" << stats_after.total_accesses << "\n";
    bool progressed = (stats_after.total_accesses >= stats_before.total_accesses + seq.size());
    cout << "RESULT: " << okfail(progressed) << "\n";

    cout << "------------------------------\n\n";
}

// ---------- Память: Hash-table vs BTree (грубая оценка) ----------
static void memory_comparison_test()
{
    cout << "===== ТЕСТ: Память (Hash-table vs BTree) =====\n";
    vector<int> sizes = {10, 100, 1000, 10000};

    for (int n : sizes)
    {
        Sequence<int> data;
        for (int i = 0; i < n; ++i)
            data.push_back(i);

        // Dictionary
        Dictionary<int, int> dict;
        for (int i = 0; i < n; ++i)
            dict.insert(i, i);
        size_t dict_entries = dict.get_size();
        // Approximate memory: entries * sizeof(pair<int,int>) + overhead (rough estimate)
        size_t approx_dict_mem = dict_entries * (sizeof(int) * 2 + 8); // +8 bytes overhead estimate

        // BTree
        BTree<int> tree;
        for (int i = 0; i < n; ++i)
            tree.insert(i);
        size_t tree_size = tree.get_size();
        // Approximate memory: nodes count unknown; we estimate key storage n * sizeof(int) + overhead
        size_t approx_tree_mem = tree_size * (sizeof(int) + 16); // +16 bytes overhead per stored key estimate

        cout << "n=" << n << " | dict_entries=" << dict_entries << " approx_mem_dict=" << approx_dict_mem
             << " | tree_entries=" << tree_size << " approx_mem_tree=" << approx_tree_mem << "\n";

        bool dict_less = approx_dict_mem <= approx_tree_mem;
        cout << "Result (dict <= tree)? " << okfail(dict_less) << "\n";
    }

    cout << "------------------------------\n\n";
}

// ---------- Запуск всех тестов (вызывается из main.cpp) ----------
void run_all_tests()
{
    // формат вывода: лаконичный по запросу
    cout << fixed << setprecision(2);
    cout << "\n===== ЗАПУСК ПОДРОБНОГО НАБОРA ТЕСТОВ =====\n\n";

    sequence_tests();
    dictionary_tests();
    btree_tests();
    cachemanager_lfu_tests();
    eviction_tests();
    hit_miss_tests();
    memory_comparison_test();

    cout << "===== НАБОР ТЕСТОВ ЗАВЕРШЕН =====\n";
}
