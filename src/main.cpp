#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

struct MemoryStats {
    long long totalKb = -1;
    long long availableKb = -1;
};

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
        } 
        else if (name == "MemAvailable:") {
            stats.availableKb = value;
        }
        if (stats.totalKb != -1 && stats.availableKb != -1) {
            break;
        }
    }

    return !file.bad()
        && stats.totalKb > 0
        && stats.availableKb >= 0
        && stats.availableKb <= stats.totalKb;
}

int main() {
    MemoryStats stats;
    if (!readMemoryStats(stats)) {
        std::cerr << "Failed to read valid memory statistics\n";
        return 1;
    }

    double usagePercent = 100.0 * (stats.totalKb - stats.availableKb) / stats.totalKb;

    std::cout << "Total: " << stats.totalKb << " kB\n";
    std::cout << "Available: " << stats.availableKb << " kB\n";
    std::cout << std::fixed << std::setprecision(2)
              << "Memory usage: " << usagePercent << "%\n";

    return 0;
}