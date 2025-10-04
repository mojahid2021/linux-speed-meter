
#ifndef SPEED_MONITOR_H
#define SPEED_MONITOR_H

#include <string>
#include <atomic>
#include <thread>
#include <mutex>
#include <chrono>

enum class SpeedUnit { KB, MB };

struct NetStats {
    uint64_t rx_bytes;
    uint64_t tx_bytes;
};

class SpeedMeter {
public:
    SpeedMeter();
    ~SpeedMeter();
    void on_quit();
    void update_loop();
    std::string get_iface() const { return iface; }
    double get_total_rx_mb() const { return total_rx / 1024.0 / 1024.0; }
    double get_total_tx_mb() const { return total_tx / 1024.0 / 1024.0; }
    double get_current_download_speed() const { return current_download_speed.load(); }
    double get_current_upload_speed() const { return current_upload_speed.load(); }
    std::string get_label() const;
    std::string get_tooltip() const;
private:
    std::string iface;
    std::atomic<bool> running;
    std::thread thread;
    NetStats last_stats;
    uint64_t total_rx, total_tx;
    std::atomic<double> current_download_speed;
    std::atomic<double> current_upload_speed;
    double smoothed_download_speed_;
    double smoothed_upload_speed_;
    bool first_sample_;
    std::chrono::steady_clock::time_point last_update_time_;
    mutable std::mutex label_mutex_;
    std::string label_;
    std::string tooltip_;
    void update_stats();
    std::string format_speed(double bytes_per_second) const;
};

#endif // SPEED_MONITOR_H