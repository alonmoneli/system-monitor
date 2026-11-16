#include "cpu_alert_rule.h"

std::optional<CpuEventKind> CpuAlertRule::update(double cpuUsage) {
    if (cpuUsage >= threshold) {
        if (!alertActive && ++consecutiveHigh == samplesToAlert) {
            alertActive = true;
            return CpuEventKind::Alert;
        }
    } else {
        consecutiveHigh = 0;
        if (alertActive) {
            alertActive = false;
            return CpuEventKind::Recovered;
        }
    }

    return std::nullopt;
}
