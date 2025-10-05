#ifndef WINDOW_H
#define WINDOW_H
#include <gtk/gtk.h>
#include <string>
#include <chrono>
#include <memory>
#include "data_manager.h"
#include "speed_test_widget.h"

class Window {
public:
    Window();
    ~Window();
    void show();
    void updateSpeeds(double uploadSpeed, double downloadSpeed, double totalUpload, double totalDownload,
                     const std::string& interface, const std::string& ip, bool connected);
    void handleClose();
    void resetStatistics();
    void exportToCSV();
    void exportToJSON();
    void setDataManager(DataManager* dm);

private:
    void createSpeedSection(GtkWidget* parent);
    void createSessionStatsSection(GtkWidget* parent);
    void createInterfaceSection(GtkWidget* parent);
    void createMonthlyStatsSection(GtkWidget* parent);
    void createDataLimitSection(GtkWidget* parent);
    void createButtonSection(GtkWidget* parent);
    void updateSessionInfo(double totalUpload, double totalDownload);
    void updateMonthlyStats();
    void formatSpeed(std::stringstream& ss, double speed, const std::string& prefix);
    std::string formatSpeedSimple(double speed);
    std::string formatBytes(double bytes);

    GtkWidget* window;
    GtkLabel* downloadLabel;
    GtkLabel* uploadLabel;
    GtkLabel* totalLabel;
    GtkLabel* sessionTimeLabel;
    GtkLabel* avgSpeedLabel;
    GtkLabel* interfaceLabel;
    GtkLabel* ipLabel;
    GtkLabel* statusLabel;

    // Monthly statistics labels
    GtkLabel* monthlyDownloadLabel;
    GtkLabel* monthlyUploadLabel;
    GtkLabel* monthlyPeakDownloadLabel;
    GtkLabel* monthlyPeakUploadLabel;
    GtkLabel* dataUsageLabel;
    GtkLabel* dataLimitStatusLabel;

    std::chrono::system_clock::time_point startTime;
    DataManager* dataManager;
    
    // Speed test widget
    std::unique_ptr<SpeedTestWidget> speedTestWidget;
    
    // Historical data for export
    struct UsageData {
        std::chrono::system_clock::time_point timestamp;
        double downloadSpeed;
        double uploadSpeed;
        double totalDownload;
        double totalUpload;
    };
    std::vector<UsageData> usageHistory;
};

#endif // WINDOW_H
