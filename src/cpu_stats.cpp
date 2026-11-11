#include "cpu_stats.h"

#include <fstream>
#include <string>

bool readCpuStats(CpuStats& stats) {
    std::ifstream file("/proc/stat");

    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    std::string label;

    if (!(file >> label >> user >> nice >> system >> idle
               >> iowait >> irq >> softirq >> steal) || label != "cpu") {
        return false;
    }

    stats.totalTicks = user + nice + system + idle + iowait + irq + softirq + steal;
    stats.idleTicks = idle + iowait;
    return true;
}
