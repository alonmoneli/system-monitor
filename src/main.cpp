#include "cpu_stats.h"
#include "memory_stats.h"
#include "cpu_alert_rule.h"

#include <curl/curl.h>
#include <string>

#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>

void sendCpuEvent(const char* kind, double cpuPercent) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Could not initialize HTTP client\n";
        return;
    }

    curl_slist* headers = curl_slist_append(nullptr, "Content-Type: application/json");
    if (!headers) {
        std::cerr << "Could not create HTTP headers\n";
        curl_easy_cleanup(curl);
        return;
    }

    const std::string body = std::string("{\"kind\":\"") + kind +
                             "\",\"cpu_percent\":" + std::to_string(cpuPercent) + "}";

    curl_easy_setopt(curl, CURLOPT_URL, "http://127.0.0.1:8000/events");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS, 1000L);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1L);

    const CURLcode result = curl_easy_perform(curl);

    if (result != CURLE_OK) {
        std::cerr << "Could not send CPU event: " << curl_easy_strerror(result) << '\n';
    } else {
        std::cout << '\n';
    }

    curl_easy_cleanup(curl);
    curl_slist_free_all(headers);
}

int main() {
    constexpr int sampleCount = 5;
    CpuStats previousCpu;
    
    if (!readCpuStats(previousCpu)) {
        std::cerr << "Failed to read initial CPU statistics\n";
        return 1;
    }

    std::cout << std::fixed << std::setprecision(2);
    CpuAlertRule alertRule;

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
        //const double cpuUsage = (sample <= 3) ? 85.0 : 20.0; - for testing.
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

        const auto event = alertRule.update(cpuUsage);

        if (event == CpuEventKind::Alert) {
            std::cout << "ALERT: sustained high CPU usage\n";
            sendCpuEvent("alert", cpuUsage);
        } 
        else if (event == CpuEventKind::Recovered) {
            std::cout << "RECOVERED: CPU usage is below the threshold\n";
            sendCpuEvent("recovered", cpuUsage);
        }
    }
    return 0;
}