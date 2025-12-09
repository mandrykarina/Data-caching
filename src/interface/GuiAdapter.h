#pragma once

#include <string>
#include <vector>
#include <cstdlib>
#include "../cache/CacheManager.h"

/**
 * @brief Adapter to export C++ cache state to Python GUI
 *
 * Usage:
 * 1. Call export_cache_state() to generate JSON
 * 2. Python visualizer reads the JSON
 * 3. Optional: implement IPC for real-time updates
 */

class GuiAdapter
{
public:
    /**
     * @brief Export cache state to JSON file
     * @tparam T Cache value type
     * @param cache Reference to CacheManager
     * @param filename Output JSON file path
     */
    template <typename T>
    static void export_cache_state(const CacheManager<T> &cache, const std::string &filename = "cache_state.json")
    {
        std::string json_str = "{\"cache_state\":[";

        // Get cache keys
        auto keys = cache.get_cache_keys();
        auto stats = cache.get_statistics();

        // Add cache entries
        for (size_t i = 0; i < keys.get_size(); ++i)
        {
            int key = static_cast<int>(keys[i]);
            auto entry = cache.get_cache_entry(key);

            if (entry)
            {
                if (i > 0)
                    json_str += ",";

                json_str += "{";
                json_str += "\"key\":" + std::to_string(key) + ",";
                json_str += "\"frequency\":" + std::to_string(entry->access_count) + ",";
                json_str += "\"last_access\":\"" + get_timestamp() + "\"";
                json_str += "}";
            }
        }

        json_str += "],";
        json_str += "\"statistics\":{";
        json_str += "\"hits\":" + std::to_string(stats.hits) + ",";
        json_str += "\"misses\":" + std::to_string(stats.misses) + ",";
        json_str += "\"hit_rate\":" + std::to_string(stats.hit_rate) + ",";
        json_str += "\"evictions\":" + std::to_string(stats.evictions) + ",";
        json_str += "\"cache_size\":" + std::to_string(cache.get_cache_size()) + ",";
        json_str += "\"max_size\":" + std::to_string(cache.get_max_cache_size());
        json_str += "}";
        json_str += "}";

        // Write to file
        FILE *file = fopen(filename.c_str(), "w");
        if (file)
        {
            fprintf(file, "%s", json_str.c_str());
            fclose(file);
        }
    }

    /**
     * @brief Launch Python GUI in separate process
     * @param gui_script Path to visualizer.py
     * @return true if GUI launched successfully
     */
    static bool launch_gui(const std::string &gui_script = "gui/visualizer.py")
    {
        std::string command;

#ifdef _WIN32
        command = "start python " + gui_script;
#else
        command = "python3 " + gui_script + " &";
#endif

        int result = system(command.c_str());
        return result == 0;
    }

    /**
     * @brief Get current timestamp as string (ISO 8601)
     */
    static std::string get_timestamp()
    {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        char buffer[30];
        strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", localtime(&time));
        return std::string(buffer);
    }
};

/**
 * Usage in main.cpp:
 *
 * #include "src/interface/GuiAdapter.h"
 *
 * int main() {
 *     CacheManager<int> cache(8);
 *
 *     // ... use cache ...
 *
 *     // Export state for visualization
 *     GuiAdapter::export_cache_state(cache, "cache_state.json");
 *
 *     // Launch Python GUI
 *     GuiAdapter::launch_gui("gui/visualizer.py");
 *
 *     return 0;
 * }
 */