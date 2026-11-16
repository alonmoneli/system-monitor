#include "cpu_alert_rule.h"

#include <iostream>

int main() {
    struct Step {
        double cpu;
        std::optional<CpuEventKind> expected;
    };

    const Step steps[] = {
        {85, std::nullopt},
        {20, std::nullopt},             // Breaks the first streak.
        {80, std::nullopt},
        {80, std::nullopt},
        {80, CpuEventKind::Alert},      // Third consecutive high sample.
        {95, std::nullopt},             // No duplicate alert.
        {20, CpuEventKind::Recovered},
        {20, std::nullopt},             // No duplicate recovery.
    };

    CpuAlertRule rule;
    int sample = 0;

    for (const auto& step : steps) {
        ++sample;
        if (rule.update(step.cpu) != step.expected) {
            std::cerr << "Unexpected event at sample " << sample << '\n';
            return 1;
        }
    }

    return 0;
}