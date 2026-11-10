#include "memory_stats.h"

#include <fstream>
#include <sstream>
#include <string>

bool readMemoryStats(MemoryStats& stats) {
    stats = MemoryStats{};

    std::ifstream file("/proc/meminfo");

    if (!file.is_open()) {
        return false;
    }

    std::string line;

    while (std::getline(file, line)) {
        std::istringstream fields(line);
        std::string name;
        long long value;

        if (!(fields >> name >> value)) {
            continue;
        }

        if (name == "MemTotal:") {
            stats.totalKb = value;
        } else if (name == "MemAvailable:") {
            stats.availableKb = value;
        }
    }

    return !file.bad()
        && stats.totalKb > 0
        && stats.availableKb >= 0
        && stats.availableKb <= stats.totalKb;
}
