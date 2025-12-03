#include <iostream>
#include "../cache/CacheManager.h"
#include "../data_structures/Sequence.h"
#include "Interactive.h"

using namespace std;

void run_interactive()
{
    cout << "\n======= INTERACTIVE CACHE DEMO =======\n";

    int cache_size = 0, data_size = 0;
    cout << "Cache size (e.g. 50): ";
    if (!(cin >> cache_size))
        return;
    cout << "Data size (e.g. 1000): ";
    if (!(cin >> data_size))
        return;
    if (cache_size <= 0 || data_size <= 0)
    {
        cout << "Invalid sizes\n";
        return;
    }

    Sequence<int> data;
    for (int i = 0; i < data_size; ++i)
        data.push_back(i);

    CacheManager<int> cache(static_cast<size_t>(cache_size));
    cache.initialize(data);

    while (true)
    {
        cout << "\n1. Request element\n2. Show statistics\n3. Show cache keys\n0. Exit\nChoice: ";
        int cmd;
        if (!(cin >> cmd))
        {
            cin.clear();
            string dummy;
            getline(cin, dummy);
            continue;
        }

        if (cmd == 1)
        {
            int key;
            cout << "Key: ";
            cin >> key;
            auto v = cache.get(key);
            if (v)
                cout << "Value: " << *v << "\n";
            else
                cout << "Not found\n";
        }
        else if (cmd == 2)
        {
            auto s = cache.get_statistics();
            cout << "\nHits: " << s.hits << "\nMisses: " << s.misses
                 << "\nHit rate: " << s.hit_rate << "%\nEvictions: " << s.evictions << "\n";
        }
        else if (cmd == 3)
        {
            auto keys = cache.get_cache_keys();
            cout << "Cached keys (" << keys.get_size() << "): ";
            for (size_t i = 0; i < keys.get_size(); ++i)
                cout << keys[i] << (i + 1 < keys.get_size() ? ", " : "\n");
        }
        else
            break;
    }
}
