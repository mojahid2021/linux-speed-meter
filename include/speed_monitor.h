
#ifndef SPEED_MONITOR_H
#define SPEED_MONITOR_H

#include <string>
#include <atomic>
#include <thread>

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
    double get_current_download_speed() const { return current_download_speed; }
    double get_current_upload_speed() const { return current_upload_speed; }
    std::string get_label() const { return label; }
    std::string get_tooltip() const { return tooltip; }
private:
    std::string iface;
    std::atomic<bool> running;
    std::thread thread;
    NetStats last_stats;
    uint64_t total_rx, total_tx;
    double current_download_speed, current_upload_speed;
    std::string label;
    std::string tooltip;
    void update_stats();
};

#endif // SPEED_MONITOR_H