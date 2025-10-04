#ifndef SPEED_TEST_WIDGET_H
#define SPEED_TEST_WIDGET_H

#include <gtk/gtk.h>
#include <memory>
#include <string>
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
    GtkWidget* container_;
    GtkWidget* serverCombo_;
    GtkWidget* startButton_;
    GtkWidget* stopButton_;
    GtkWidget* progressBar_;
    GtkWidget* statusLabel_;
    
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
    
    // Signal handlers
    static void onStartClicked(GtkButton* button, gpointer userData);
    static void onStopClicked(GtkButton* button, gpointer userData);
    
    // Test execution
    void startTest();
    void stopTest();
    void runTestAsync();
    
    // UI helpers
    void setTestRunning(bool running);
    std::string formatSpeed(double mbps);
    std::string formatPing(double ms);
};

#endif // SPEED_TEST_WIDGET_H
