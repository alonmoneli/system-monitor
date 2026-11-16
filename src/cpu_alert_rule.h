#pragma once

#include <optional>

enum class CpuEventKind { Alert, Recovered };

class CpuAlertRule {
    
private:
    static constexpr double threshold = 80.0;
    static constexpr int samplesToAlert = 3;
    int consecutiveHigh = 0;
    bool alertActive = false;

public:
    std::optional<CpuEventKind> update(double cpuUsage);
};
