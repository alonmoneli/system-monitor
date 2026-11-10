#ifndef SYSTEM_MONITOR_MEMORY_STATS_H
#define SYSTEM_MONITOR_MEMORY_STATS_H

struct MemoryStats {
    long long totalKb = -1;
    long long availableKb = -1;
};

bool readMemoryStats(MemoryStats& stats);

#endif
