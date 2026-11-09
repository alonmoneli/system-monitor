#include <fstream>
#include <iostream>
#include <string>

int main() {
    std::ifstream file("/proc/meminfo");
    if (!file.is_open()) {
        std::cerr << "Failed to open /proc/meminfo\n";
        return 1;
    }
    std::string line;
    for (int i = 0; i < 5 && std::getline(file, line); ++i) {
        std::cout << line << '\n';
    }
    return 0;
}