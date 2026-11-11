#ifndef SYSTEM_MONITOR_CPU_STATS_H
#define SYSTEM_MONITOR_CPU_STATS_H

struct CpuStats {
    unsigned long long totalTicks = 0;
    unsigned long long idleTicks = 0;
};

bool readCpuStats(CpuStats& stats);

#endif
