#include <iostream>
#include "src/tests/test_all.h"
#include "src/benchmark/Benchmark.h"
#include "src/interface/Interactive.h"

using namespace std;

int main()
{
    while (true)
    {
        cout << "\n======================================\n";
        cout << "           CACHE SYSTEM MENU           \n";
        cout << "======================================\n";
        cout << "1. Run unit tests\n";
        cout << "2. Run benchmarks\n";
        cout << "3. Interactive mode\n";
        cout << "0. Exit\n";
        cout << "Select: ";

        int opt;
        if (!(cin >> opt))
        {
            cin.clear();
            string dummy;
            getline(cin, dummy);
            continue;
        }

        if (opt == 1)
            run_all_tests();
        else if (opt == 2)
            run_all_benchmarks();
        else if (opt == 3)
            run_interactive();
        else
            break;
    }
    return 0;
}
