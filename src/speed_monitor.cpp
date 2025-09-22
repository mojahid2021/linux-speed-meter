
#include "../include/speed_monitor.h"
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <algorithm>
#include <iostream>


constexpr int UPDATE_INTERVAL = 1; // seconds
constexpr SpeedUnit DISPLAY_UNIT = SpeedUnit::KB;

// Improved interface detection: prefer non-loopback, up, and with traffic
static std::string get_active_interface() {
    std::ifstream route("/proc/net/route");
    if (!route.is_open()) {
        std::cerr << "Error: Cannot open /proc/net/route" << std::endl;
        return "eth0";
    }
    std::string line, iface;
    while (std::getline(route, line)) {
        std::istringstream iss(line);
        std::string ifname, destination;
        iss >> ifname >> destination;
        if (destination == "00000000" && ifname != "lo") {
            iface = ifname;
            break;
        }
    }
    if (!iface.empty()) return iface;
    // Fallback: first non-loopback interface in /proc/net/dev
    std::ifstream netdev("/proc/net/dev");
    if (!netdev.is_open()) {
        std::cerr << "Error: Cannot open /proc/net/dev" << std::endl;
        return "eth0";
    }
    while (std::getline(netdev, line)) {
        size_t pos = line.find(":");
        if (pos != std::string::npos) {
            std::string name = line.substr(0, pos);
            name.erase(std::remove_if(name.begin(), name.end(), ::isspace), name.end());
            if (name != "lo" && !name.empty()) return name;
        }
    }
    return "eth0"; // last fallback
}

static NetStats get_net_stats(const std::string& iface) {
    std::ifstream netdev("/proc/net/dev");
    if (!netdev.is_open()) {
        std::cerr << "Error: Cannot open /proc/net/dev for interface " << iface << std::endl;
        return {0, 0};
    }
    std::string line;
    while (std::getline(netdev, line)) {
        if (line.find(iface + ":") != std::string::npos) {
            size_t colon = line.find(":");
            std::istringstream iss(line.substr(colon + 1));
            NetStats stats;
            iss >> stats.rx_bytes;
            for (int i = 0; i < 7; ++i) iss >> line; // skip fields
            iss >> stats.tx_bytes;
            return stats;
        }
    }
    std::cerr << "Warning: Interface " << iface << " not found in /proc/net/dev" << std::endl;
    return {0, 0};
}

static std::string format_speed(uint64_t bytes) {
    std::ostringstream oss;
    double bits = bytes * 8.0;
    double kbits = bits / 1024.0;
    oss << std::fixed;
    if (kbits < 1024.0) {
        oss.precision(1);
        oss << kbits << " Kb/s";
    } else {
        oss.precision(2);
        oss << (kbits / 1024.0) << " Mb/s";
    }
    return oss.str();
}

SpeedMeter::SpeedMeter()
    : running(true), total_rx(0), total_tx(0) {
    iface = get_active_interface();
    if (iface.empty()) {
        throw std::runtime_error("No active network interface found.");
    }
    std::cout << "Monitoring interface: " << iface << std::endl;
    last_stats = get_net_stats(iface);
    thread = std::thread(&SpeedMeter::update_loop, this);
}

SpeedMeter::~SpeedMeter() {
    running = false;
    if (thread.joinable()) thread.join();
}

void SpeedMeter::on_quit() {
    running = false;
}

void SpeedMeter::update_loop() {
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(UPDATE_INTERVAL));
        update_stats();
    }
}

void SpeedMeter::update_stats() {
    auto curr_stats = get_net_stats(iface);
    uint64_t rx = curr_stats.rx_bytes - last_stats.rx_bytes;
    uint64_t tx = curr_stats.tx_bytes - last_stats.tx_bytes;
    total_rx += rx;
    total_tx += tx;
    last_stats = curr_stats;

    label = "↓ " + format_speed(rx) + " | ↑ " + format_speed(tx);
    std::ostringstream tooltip_oss;
    tooltip_oss.precision(2);
    tooltip_oss << std::fixed;
    tooltip_oss << "Interface: " << iface << "\nTotal Download: "
                << (total_rx / 1024.0 / 1024.0) << " MB\nTotal Upload: "
                << (total_tx / 1024.0 / 1024.0) << " MB";
    tooltip = tooltip_oss.str();

    // Debug output
    std::cout << "[DEBUG] Interface: " << iface
              << " | RX: " << rx << " bytes | TX: " << tx << " bytes"
              << " | Label: " << label << std::endl;
}
