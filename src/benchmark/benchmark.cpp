// src/benchmark/benchmark.cpp

#include <iostream>
#include <fstream>
#include <chrono>
#include <iomanip>

#include "../data_structures/Sequence.h"
#include "../data_structures/Dictionary.h"
#include "../data_structures/BTree.h"

using namespace std;

// Таймер
long long ms_now()
{
    return chrono::duration_cast<chrono::milliseconds>(
               chrono::steady_clock::now().time_since_epoch())
        .count();
}

// ------------------------
// Оценка памяти
// ------------------------
size_t estimate_memory_dictionary(size_t entries)
{
    // приблизительная модель:
    //   Pair(K,V) ≈ 16–24 bytes (ключ + значение + выравнивание)
    //   указатели списков, bucket array etc
    //   грубо: 48 bytes на элемент
    return entries * 48;
}

size_t estimate_memory_btree(size_t entries)
{
    // в BTree каждый узел содержит до ORDER-1 ключей и ORDER детей
    // ORDER=4, значит 3 ключа, 4 указателя
    // Эмпирическая модель: ~64 bytes на запись
    return entries * 64;
}

// ------------------------
// Бенчмарк для Dictionary и BTree
// ------------------------
struct BenchResult
{
    int n;
    long long insert_dict_ms;
    long long search_dict_ms;
    long long insert_tree_ms;
    long long search_tree_ms;
    size_t mem_dict;
    size_t mem_tree;
};

BenchResult run_single(int n)
{
    BenchResult r;
    r.n = n;

    Sequence<int> data;

    for (int i = 0; i < n; ++i)
        data.push_back(i);

    // ---------------------------
    // Dictionary — вставка
    // ---------------------------
    Dictionary<int, int> dict;

    long long t1 = ms_now();
    for (int i = 0; i < n; ++i)
        dict.insert(i, i);
    long long t2 = ms_now();
    r.insert_dict_ms = t2 - t1;

    // поиск 1000 случайных значений или всех n если n<1000
    t1 = ms_now();
    for (int i = 0; i < min(n, 1000); ++i)
    {
        int key = i % n;
        dict.find(key);
    }
    t2 = ms_now();
    r.search_dict_ms = t2 - t1;

    // ---------------------------
    // BTree — вставка
    // ---------------------------
    BTree<int> tree;

    t1 = ms_now();
    for (int i = 0; i < n; ++i)
        tree.insert(i);
    t2 = ms_now();
    r.insert_tree_ms = t2 - t1;

    // ---------------------------
    // поиск
    // ---------------------------
    t1 = ms_now();
    for (int i = 0; i < min(n, 1000); ++i)
    {
        int key = i % n;
        tree.search(key);
    }
    t2 = ms_now();
    r.search_tree_ms = t2 - t1;

    // ---------------------------
    // Память
    // ---------------------------
    r.mem_dict = estimate_memory_dictionary(dict.get_size());
    r.mem_tree = estimate_memory_btree(tree.get_size());

    return r;
}

// Запуск всех тестов
void run_all_benchmarks()
{
    cout << "\n=========== BENCHMARK: HashTable vs BTree ===========\n";

    vector<int> sizes = {10, 100, 1000, 10000};
    vector<BenchResult> results;

    for (int n : sizes)
    {
        cout << "\n--- Размер: " << n << " ---\n";
        BenchResult r = run_single(n);

        cout << "Dictionary insert: " << r.insert_dict_ms << " ms\n";
        cout << "Dictionary search: " << r.search_dict_ms << " ms\n";
        cout << "BTree insert:      " << r.insert_tree_ms << " ms\n";
        cout << "BTree search:      " << r.search_tree_ms << " ms\n";
        cout << "Approx memory Dictionary: " << r.mem_dict << " bytes\n";
        cout << "Approx memory BTree:      " << r.mem_tree << " bytes\n";

        results.push_back(r);
    }

    // ---------------------------------------------
    // Выгрузка CSV: скорость
    // ---------------------------------------------
    {
        ofstream out("benchmark_speed.csv");
        out << "n,dict_insert_ms,dict_search_ms,btree_insert_ms,btree_search_ms\n";
        for (auto &r : results)
        {
            out << r.n << ","
                << r.insert_dict_ms << ","
                << r.search_dict_ms << ","
                << r.insert_tree_ms << ","
                << r.search_tree_ms << "\n";
        }
    }

    // ---------------------------------------------
    // Выгрузка CSV: память
    // ---------------------------------------------
    {
        ofstream out("benchmark_memory.csv");
        out << "n,dict_memory,btree_memory\n";
        for (auto &r : results)
        {
            out << r.n << ","
                << r.mem_dict << ","
                << r.mem_tree << "\n";
        }
    }

    // ---------------------------------------------
    // Консольная итоговая таблица
    // ---------------------------------------------
    cout << "\n=========== Итоговая таблица ===========\n";
    cout << left << setw(10) << "n"
         << setw(18) << "dict_ins (ms)"
         << setw(18) << "dict_search (ms)"
         << setw(18) << "tree_ins (ms)"
         << setw(18) << "tree_search (ms)"
         << setw(18) << "dict_mem"
         << setw(18) << "tree_mem"
         << "\n";

    for (auto &r : results)
    {
        cout << left << setw(10) << r.n
             << setw(18) << r.insert_dict_ms
             << setw(18) << r.search_dict_ms
             << setw(18) << r.insert_tree_ms
             << setw(18) << r.search_tree_ms
             << setw(18) << r.mem_dict
             << setw(18) << r.mem_tree
             << "\n";
    }

    cout << "\nCSV файлы созданы: benchmark_speed.csv, benchmark_memory.csv\n";
    cout << "=========== BENCHMARK FINISHED ===========\n\n";
}
