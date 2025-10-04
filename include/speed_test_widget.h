#ifndef SPEED_TEST_WIDGET_H
#define SPEED_TEST_WIDGET_H

#include <gtk/gtk.h>
#include <memory>
#include <string>
#include <deque>
#include <thread>
#include <atomic>
#include <functional>
#include "speed_test.h"
#include "download_test.h"
#include "upload_test.h"
#include "ping_test.h"

// GTK widget for speed test interface
class SpeedTestWidget {
public:
    SpeedTestWidget();
    ~SpeedTestWidget();
    
    // Create the widget and return container
    GtkWidget* create();
    
    // Update display during test
    void updateProgress(const std::string& stage, double progress, double currentSpeed);
    
    // Update results after test
    void updateResults(const SpeedTestResult& result);
    
private:
    struct TestRunConfig {
        int durationSeconds;
        int warmupSeconds;
        int parallelConnections;
        bool autoSelect;
        int serverIndex;
    };

    GtkWidget* container_;
    GtkWidget* serverCombo_;
    GtkWidget* startButton_;
    GtkWidget* stopButton_;
    GtkWidget* progressBar_;
    GtkWidget* statusLabel_;
    GtkWidget* durationSpin_;
    GtkWidget* warmupSpin_;
    GtkWidget* connectionSpin_;
    GtkWidget* autoSelectCheck_;
    GtkWidget* historyList_;
    
    // Result labels
    GtkWidget* downloadLabel_;
    GtkWidget* uploadLabel_;
    GtkWidget* pingLabel_;
    GtkWidget* jitterLabel_;
    
    // Test instances
    std::unique_ptr<SpeedTest> speedTest_;
    std::unique_ptr<DownloadTest> downloadTest_;
    std::unique_ptr<UploadTest> uploadTest_;
    std::unique_ptr<PingTest> pingTest_;
    
    std::vector<TestServer> servers_;
    bool testRunning_;
    std::deque<SpeedTestResult> history_;
    
    // Signal handlers
    static void onStartClicked(GtkButton* button, gpointer userData);
    static void onStopClicked(GtkButton* button, gpointer userData);
    
    // Test execution
    void startTest();
    void stopTest();
    void runTestAsync(TestRunConfig config);
    TestServer pickServer(const TestRunConfig& config);
    void refreshHistory();
    void handleSettingsChange();
    int getDurationSeconds() const;
    int getWarmupSeconds() const;
    int getParallelConnections() const;
    
    // UI helpers
    void setTestRunning(bool running);
    std::string formatSpeed(double mbps);
    std::string formatPing(double ms);

    // Worker helpers
    void joinWorker();
    void handleCancellation();
    void dispatchToUi(std::function<void(SpeedTestWidget&)> fn);
    bool isCancelled() const;

    std::thread workerThread_;
    std::atomic<bool> cancelRequested_;
    std::atomic<bool> workerActive_;
    std::shared_ptr<void> aliveToken_;
    mutable std::mutex uiMutex_;  // Protect UI access from worker thread
};

#endif // SPEED_TEST_WIDGET_H
