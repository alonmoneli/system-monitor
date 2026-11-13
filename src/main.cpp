#include "cpu_stats.h"
#include "memory_stats.h"

#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

int main() {
    constexpr int sampleCount = 5;
    constexpr double cpuThreshold = 80.0;
    constexpr int samplesToAlert = 3;
    
    int consecutiveHigh = 0;
    bool alertActive = false;

    CpuStats previousCpu;
    if (!readCpuStats(previousCpu)) {
        std::cerr << "Failed to read initial CPU statistics\n";
        return 1;
    }

    std::cout << std::fixed << std::setprecision(2);

    for (int sample = 1; sample <= sampleCount; ++sample) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        CpuStats currentCpu;
        if (!readCpuStats(currentCpu)) {
            std::cerr << "Failed to read CPU statistics\n";
            return 1;
        }

        if (currentCpu.totalTicks <= previousCpu.totalTicks ||
            currentCpu.idleTicks < previousCpu.idleTicks) {
            std::cerr << "Invalid CPU counter interval\n";
            return 1;
        }

        const auto totalDelta = currentCpu.totalTicks - previousCpu.totalTicks;
        const auto idleDelta = currentCpu.idleTicks - previousCpu.idleTicks;

        if (idleDelta > totalDelta) {
            std::cerr << "Idle ticks exceed total ticks for this interval\n";
            return 1;
        }

        const double cpuUsage = 100.0 * (totalDelta - idleDelta) / totalDelta;
        previousCpu = currentCpu;

        MemoryStats memory;
        if (!readMemoryStats(memory)) {
            std::cerr << "Failed to read memory statistics\n";
            return 1;
        }

        const double memoryUsage =
            100.0 * (memory.totalKb - memory.availableKb) / memory.totalKb;

        std::cout << "Sample " << sample << " | CPU: " << cpuUsage << "%" 
        << " | Memory: " << memoryUsage << "%\n";

        if (cpuUsage >= cpuThreshold) {
            ++consecutiveHigh;
            if (consecutiveHigh == samplesToAlert) {
                alertActive = true;
                std::cout << "ALERT: sustained high CPU usage\n";
            }
        } 
        else {
            consecutiveHigh = 0;
            if (alertActive) {
                alertActive = false;
                std::cout << "RECOVERED: CPU usage is below the threshold\n";
            }
        }
    }

    return 0;
}