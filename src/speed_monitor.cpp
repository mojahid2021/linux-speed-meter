
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

SpeedMeter::SpeedMeter()
    : running(true),
      total_rx(0),
      total_tx(0),
      current_download_speed(0.0),
      current_upload_speed(0.0),
      smoothed_download_speed_(0.0),
      smoothed_upload_speed_(0.0),
      first_sample_(true) {
    iface = get_active_interface();
    if (iface.empty()) {
        throw std::runtime_error("No active network interface found.");
    }
    std::cout << "Monitoring interface: " << iface << std::endl;
    last_stats = get_net_stats(iface);
    last_update_time_ = std::chrono::steady_clock::now();
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
    auto now = std::chrono::steady_clock::now();
    double elapsed_seconds = std::chrono::duration<double>(now - last_update_time_).count();
    if (elapsed_seconds <= 0.0) {
        elapsed_seconds = static_cast<double>(UPDATE_INTERVAL);
    }
    last_update_time_ = now;

    uint64_t rx = curr_stats.rx_bytes - last_stats.rx_bytes;
    uint64_t tx = curr_stats.tx_bytes - last_stats.tx_bytes;
    total_rx += rx;
    total_tx += tx;
    last_stats = curr_stats;

    // Calculate instantaneous speeds (bytes per second)
    double instant_download = static_cast<double>(rx) / elapsed_seconds;
    double instant_upload = static_cast<double>(tx) / elapsed_seconds;

    constexpr double alpha = 0.6; // smoothing factor for EMA
    if (first_sample_) {
        smoothed_download_speed_ = instant_download;
        smoothed_upload_speed_ = instant_upload;
        first_sample_ = false;
    } else {
        smoothed_download_speed_ = alpha * instant_download + (1.0 - alpha) * smoothed_download_speed_;
        smoothed_upload_speed_ = alpha * instant_upload + (1.0 - alpha) * smoothed_upload_speed_;
    }

    current_download_speed.store(smoothed_download_speed_);
    current_upload_speed.store(smoothed_upload_speed_);

    std::ostringstream tooltip_oss;
    tooltip_oss.precision(2);
    tooltip_oss << std::fixed;
    tooltip_oss << "Interface: " << iface << "\nTotal Download: "
                << (total_rx / 1024.0 / 1024.0) << " MB\nTotal Upload: "
                << (total_tx / 1024.0 / 1024.0) << " MB";
    const std::string tooltip = tooltip_oss.str();

    const std::string label = "↓ " + format_speed(smoothed_download_speed_) +
                              " | ↑ " + format_speed(smoothed_upload_speed_);

    {
        std::lock_guard<std::mutex> lock(label_mutex_);
        label_ = label;
        tooltip_ = tooltip;
    }

    // Debug output
    std::cout << "[DEBUG] Interface: " << iface
              << " | RX: " << rx << " bytes | TX: " << tx << " bytes"
              << " | Instant Down: " << instant_download << " B/s"
              << " | Instant Up: " << instant_upload << " B/s"
              << " | Smoothed Down: " << smoothed_download_speed_
              << " B/s | Smoothed Up: " << smoothed_upload_speed_
              << " B/s | Label: " << label << std::endl;
}

std::string SpeedMeter::format_speed(double bytes) const {
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

std::string SpeedMeter::get_label() const {
    std::lock_guard<std::mutex> lock(label_mutex_);
    return label_;
}

std::string SpeedMeter::get_tooltip() const {
    std::lock_guard<std::mutex> lock(label_mutex_);
    return tooltip_;
}
