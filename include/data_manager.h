#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H

#include <string>
#include <vector>
#include <chrono>
#include <map>
#include <fstream>
#include <sstream>
#include <iomanip>

struct DailyStats {
    std::string date;  // YYYY-MM-DD format
    uint64_t total_download_bytes;
    uint64_t total_upload_bytes;
    double peak_download_speed;  // bytes per second
    double peak_upload_speed;    // bytes per second
    uint64_t session_count;
    std::chrono::seconds total_session_time;
};

struct MonthlyStats {
    std::string month;  // YYYY-MM format
    uint64_t total_download_bytes;
    uint64_t total_upload_bytes;
    double peak_download_speed;
    double peak_upload_speed;
    double avg_daily_download;
    double avg_daily_upload;
    uint32_t active_days;
};

class DataManager {
public:
    DataManager(const std::string& data_dir = "~/.config/linux-speed-meter");
    ~DataManager();

    // Daily statistics
    void updateDailyStats(uint64_t download_bytes, uint64_t upload_bytes,
                         double current_download_speed, double current_upload_speed,
                         std::chrono::seconds session_time);
    DailyStats getTodayStats() const;
    DailyStats getDailyStats(const std::string& date) const;
    std::vector<DailyStats> getDailyStatsRange(const std::string& start_date,
                                              const std::string& end_date) const;

    // Monthly statistics
    MonthlyStats getMonthlyStats(const std::string& month) const;
    std::vector<MonthlyStats> getMonthlyStatsRange(const std::string& start_month,
                                                  const std::string& end_month) const;
    MonthlyStats getCurrentMonthStats() const;

    // Data limits and alerts
    void setDataLimit(uint64_t monthly_limit_bytes);
    uint64_t getDataLimit() const;
    double getDataUsagePercentage() const;
    bool isDataLimitExceeded() const;

    // Utility functions
    void saveData();
    void loadData();
    void resetMonthlyData();
    void exportData(const std::string& filename) const;

private:
    std::string data_file_path;
    std::map<std::string, DailyStats> daily_stats;
    uint64_t monthly_data_limit;

    std::string getCurrentDate() const;
    std::string getCurrentMonth() const;
    std::string expandPath(const std::string& path) const;
    void ensureDataDirectory();
    void calculateMonthlyStats();
};

#endif // DATA_MANAGER_H