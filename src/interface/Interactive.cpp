#include <iostream>
#include "../cache/CacheManager.h"
#include "../data_structures/Sequence.h"
#include "Interactive.h"

using namespace std;

void run_interactive()
{
    cout << "\n======= INTERACTIVE CACHE DEMO =======\n";

    int cache_size, data_size;
    cout << "Cache size: ";
    cin >> cache_size;
    cout << "Data size: ";
    cin >> data_size;

    Sequence<int> data;
    for (int i = 0; i < data_size; i++)
        data.push_back(i);

    CacheManager<int> cache(cache_size);
    cache.initialize(data);

    while (true)
    {
        cout << "\n1. Request element\n";
        cout << "2. Show statistics\n";
        cout << "0. Exit\n";
        cout << "Choice: ";

        int cmd;
        cin >> cmd;

        if (cmd == 1)
        {
            int key;
            cout << "Key: ";
            cin >> key;

            auto res = cache.get(key);
            if (res)
                cout << "Value: " << *res << "\n";
            else
                cout << "Not found\n";
        }
        else if (cmd == 2)
        {
            auto s = cache.get_statistics();
            cout << "\nHits: " << s.hits;
            cout << "\nMisses: " << s.misses;
            cout << "\nHit rate: " << s.hit_rate << "%\n";
        }
        else
            break;
    }
}
