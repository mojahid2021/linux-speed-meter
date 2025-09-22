#include "../include/data_manager.h"
#include <iostream>
#include <algorithm>
#include <ctime>
#include <set>
#include <sys/stat.h>
#include <unistd.h>

DataManager::DataManager(const std::string& data_dir)
    : monthly_data_limit(0) {
    data_file_path = expandPath(data_dir) + "/usage_data.txt";
    ensureDataDirectory();
    loadData();
}

DataManager::~DataManager() {
    saveData();
}

std::string DataManager::expandPath(const std::string& path) const {
    if (path.empty() || path[0] != '~') {
        return path;
    }

    const char* home = getenv("HOME");
    if (!home) {
        return path;
    }

    return std::string(home) + path.substr(1);
}

void DataManager::ensureDataDirectory() {
    // Create directory if it doesn't exist (simplified version)
    std::string dir_path = data_file_path.substr(0, data_file_path.find_last_of('/'));
    if (access(dir_path.c_str(), F_OK) != 0) {
        // Try to create directory
        std::string cmd = "mkdir -p '" + dir_path + "'";
        system(cmd.c_str());
    }
}

std::string DataManager::getCurrentDate() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time_t);

    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%d");
    return oss.str();
}

std::string DataManager::getCurrentMonth() const {
    auto now = std::chrono::system_clock::now();
    auto time_t = std::chrono::system_clock::to_time_t(now);
    std::tm* tm = std::localtime(&time_t);

    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m");
    return oss.str();
}

void DataManager::updateDailyStats(uint64_t download_bytes, uint64_t upload_bytes,
                                  double current_download_speed, double current_upload_speed,
                                  std::chrono::seconds session_time) {
    std::string today = getCurrentDate();

    if (daily_stats.find(today) == daily_stats.end()) {
        daily_stats[today] = DailyStats{today, 0, 0, 0.0, 0.0, 0, std::chrono::seconds(0)};
    }

    auto& stats = daily_stats[today];
    stats.total_download_bytes += download_bytes;
    stats.total_upload_bytes += upload_bytes;
    stats.peak_download_speed = std::max(stats.peak_download_speed, current_download_speed);
    stats.peak_upload_speed = std::max(stats.peak_upload_speed, current_upload_speed);
    stats.session_count++;
    stats.total_session_time += session_time;

    saveData();
}

DailyStats DataManager::getTodayStats() const {
    std::string today = getCurrentDate();
    auto it = daily_stats.find(today);
    if (it != daily_stats.end()) {
        return it->second;
    }
    return DailyStats{today, 0, 0, 0.0, 0.0, 0, std::chrono::seconds(0)};
}

DailyStats DataManager::getDailyStats(const std::string& date) const {
    auto it = daily_stats.find(date);
    if (it != daily_stats.end()) {
        return it->second;
    }
    return DailyStats{date, 0, 0, 0.0, 0.0, 0, std::chrono::seconds(0)};
}

std::vector<DailyStats> DataManager::getDailyStatsRange(const std::string& start_date,
                                                       const std::string& end_date) const {
    std::vector<DailyStats> result;

    for (const auto& pair : daily_stats) {
        if (pair.first >= start_date && pair.first <= end_date) {
            result.push_back(pair.second);
        }
    }

    std::sort(result.begin(), result.end(),
              [](const DailyStats& a, const DailyStats& b) { return a.date < b.date; });

    return result;
}

MonthlyStats DataManager::getMonthlyStats(const std::string& month) const {
    MonthlyStats stats{month, 0, 0, 0.0, 0.0, 0.0, 0.0, 0};

    for (const auto& pair : daily_stats) {
        if (pair.first.substr(0, 7) == month) {
            stats.total_download_bytes += pair.second.total_download_bytes;
            stats.total_upload_bytes += pair.second.total_upload_bytes;
            stats.peak_download_speed = std::max(stats.peak_download_speed, pair.second.peak_download_speed);
            stats.peak_upload_speed = std::max(stats.peak_upload_speed, pair.second.peak_upload_speed);
            stats.active_days++;
        }
    }

    if (stats.active_days > 0) {
        stats.avg_daily_download = static_cast<double>(stats.total_download_bytes) / stats.active_days;
        stats.avg_daily_upload = static_cast<double>(stats.total_upload_bytes) / stats.active_days;
    }

    return stats;
}

MonthlyStats DataManager::getCurrentMonthStats() const {
    return getMonthlyStats(getCurrentMonth());
}

std::vector<MonthlyStats> DataManager::getMonthlyStatsRange(const std::string& start_month,
                                                           const std::string& end_month) const {
    std::vector<MonthlyStats> result;
    std::set<std::string> months;

    // Collect unique months
    for (const auto& pair : daily_stats) {
        months.insert(pair.first.substr(0, 7));
    }

    for (const auto& month : months) {
        if (month >= start_month && month <= end_month) {
            result.push_back(getMonthlyStats(month));
        }
    }

    std::sort(result.begin(), result.end(),
              [](const MonthlyStats& a, const MonthlyStats& b) { return a.month < b.month; });

    return result;
}

void DataManager::setDataLimit(uint64_t monthly_limit_bytes) {
    monthly_data_limit = monthly_limit_bytes;
    saveData();
}

uint64_t DataManager::getDataLimit() const {
    return monthly_data_limit;
}

double DataManager::getDataUsagePercentage() const {
    if (monthly_data_limit == 0) return 0.0;

    MonthlyStats current = getCurrentMonthStats();
    uint64_t total_usage = current.total_download_bytes + current.total_upload_bytes;
    return (static_cast<double>(total_usage) / monthly_data_limit) * 100.0;
}

bool DataManager::isDataLimitExceeded() const {
    if (monthly_data_limit == 0) return false;

    MonthlyStats current = getCurrentMonthStats();
    uint64_t total_usage = current.total_download_bytes + current.total_upload_bytes;
    return total_usage > monthly_data_limit;
}

void DataManager::saveData() {
    std::ofstream file(data_file_path);
    if (!file.is_open()) {
        std::cerr << "Error opening data file for writing: " << data_file_path << std::endl;
        return;
    }

    file << monthly_data_limit << std::endl;

    for (const auto& pair : daily_stats) {
        const auto& stats = pair.second;
        file << stats.date << ","
             << stats.total_download_bytes << ","
             << stats.total_upload_bytes << ","
             << stats.peak_download_speed << ","
             << stats.peak_upload_speed << ","
             << stats.session_count << ","
             << stats.total_session_time.count() << std::endl;
    }
}

void DataManager::loadData() {
    std::ifstream file(data_file_path);
    if (!file.is_open()) {
        std::cout << "Data file not found, starting with empty statistics" << std::endl;
        return;
    }

    std::string line;

    // Read data limit
    if (std::getline(file, line)) {
        try {
            monthly_data_limit = std::stoull(line);
        } catch (const std::exception&) {
            monthly_data_limit = 0;
        }
    }

    // Read daily statistics
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        DailyStats stats;

        if (std::getline(iss, stats.date, ',')) {
            std::getline(iss, token, ',');
            stats.total_download_bytes = std::stoull(token);
            std::getline(iss, token, ',');
            stats.total_upload_bytes = std::stoull(token);
            std::getline(iss, token, ',');
            stats.peak_download_speed = std::stod(token);
            std::getline(iss, token, ',');
            stats.peak_upload_speed = std::stod(token);
            std::getline(iss, token, ',');
            stats.session_count = std::stoull(token);
            std::getline(iss, token, ',');
            stats.total_session_time = std::chrono::seconds(std::stoll(token));

            daily_stats[stats.date] = stats;
        }
    }

    std::cout << "Loaded " << daily_stats.size() << " days of statistics" << std::endl;
}

void DataManager::resetMonthlyData() {
    std::string current_month = getCurrentMonth();

    for (auto it = daily_stats.begin(); it != daily_stats.end(); ) {
        if (it->first.substr(0, 7) == current_month) {
            it = daily_stats.erase(it);
        } else {
            ++it;
        }
    }

    saveData();
}

void DataManager::exportData(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Error opening export file: " << filename << std::endl;
        return;
    }

    file << "Date,Download (MB),Upload (MB),Peak Download (MB/s),Peak Upload (MB/s),Sessions,Session Time (hours)" << std::endl;

    for (const auto& pair : daily_stats) {
        const auto& stats = pair.second;
        file << stats.date << ","
             << (stats.total_download_bytes / 1024.0 / 1024.0) << ","
             << (stats.total_upload_bytes / 1024.0 / 1024.0) << ","
             << (stats.peak_download_speed / 1024.0 / 1024.0) << ","
             << (stats.peak_upload_speed / 1024.0 / 1024.0) << ","
             << stats.session_count << ","
             << (stats.total_session_time.count() / 3600.0) << std::endl;
    }
}