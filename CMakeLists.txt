cmake_minimum_required(VERSION 3.20)
project(SystemMonitor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(CURL REQUIRED)

add_executable(system_monitor
    src/main.cpp
    src/memory_stats.cpp
    src/cpu_stats.cpp
    src/cpu_alert_rule.cpp
)

target_link_libraries(system_monitor PRIVATE CURL::libcurl)
target_compile_options(system_monitor PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()

add_executable(cpu_alert_rule_test
    tests/cpu_alert_rule_test.cpp
    src/cpu_alert_rule.cpp
)

target_include_directories(cpu_alert_rule_test PRIVATE src)
target_compile_options(cpu_alert_rule_test PRIVATE -Wall -Wextra -Wpedantic)

add_test(NAME cpu_alert_rule COMMAND cpu_alert_rule_test)