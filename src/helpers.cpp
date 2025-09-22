#include "../include/helpers.h"
#include <string>
#include <iomanip>
#include <sstream>

std::string formatSpeed(double speed) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << speed << " Mbps";
    return oss.str();
}

std::string formatDataUsage(long long bytes) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = static_cast<double>(bytes);

    while (size >= 1024 && unitIndex < 4) {
        size /= 1024;
        unitIndex++;
    }

    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << size << " " << units[unitIndex];
    return oss.str();
}