cmake_minimum_required(VERSION 3.20)
project(SystemMonitor LANGUAGES CXX)

add_executable(system_monitor src/main.cpp)

set_target_properties(system_monitor PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)

target_compile_options(system_monitor PRIVATE -Wall -Wextra -Wpedantic)