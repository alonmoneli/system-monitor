cmake_minimum_required(VERSION 3.20)
project(SystemMonitor LANGUAGES CXX)
find_package(CURL REQUIRED)

add_executable(system_monitor
    src/main.cpp
    src/memory_stats.cpp
    src/cpu_stats.cpp
)

target_link_libraries(system_monitor PRIVATE CURL::libcurl)

set_target_properties(system_monitor PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_compile_options(system_monitor PRIVATE -Wall -Wextra -Wpedantic)
