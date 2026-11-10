#include "memory_stats.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

int main() {
    constexpr int sampleCount = 5;

    std::cout << std::fixed << std::setprecision(2);

    for (int sample = 0; sample < sampleCount; ++sample) {
        MemoryStats stats;

        if (!readMemoryStats(stats)) {
            std::cerr << "Failed to read valid memory statistics\n";
            return 1;
        }

        double usagePercent =
            100.0 * (stats.totalKb - stats.availableKb) / stats.totalKb;

        std::cout << "Sample " << sample + 1
                  << " | Memory usage: " << usagePercent << "%\n";

        if (sample + 1 < sampleCount) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    return 0;
}