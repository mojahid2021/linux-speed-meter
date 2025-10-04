#include "../include/speed_test_widget.h"
#include <thread>
#include <sstream>
#include <iomanip>

SpeedTestWidget::SpeedTestWidget() 
    : container_(nullptr), testRunning_(false) {
    
    speedTest_ = std::make_unique<SpeedTest>();
    downloadTest_ = std::make_unique<DownloadTest>();
    uploadTest_ = std::make_unique<UploadTest>();
    pingTest_ = std::make_unique<PingTest>();
    
    servers_ = SpeedTest::getDefaultServers();
}

SpeedTestWidget::~SpeedTestWidget() {
    if (testRunning_) {
        stopTest();
    }
}

GtkWidget* SpeedTestWidget::create() {
    // Main container
    container_ = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(container_, 20);
    gtk_widget_set_margin_end(container_, 20);
    gtk_widget_set_margin_top(container_, 20);
    gtk_widget_set_margin_bottom(container_, 20);
    
    // Title
    GtkWidget* titleLabel = gtk_label_new(nullptr);
    gtk_label_set_markup(GTK_LABEL(titleLabel), 
        "<span size='x-large' weight='bold'>Internet Speed Test</span>");
    gtk_box_pack_start(GTK_BOX(container_), titleLabel, FALSE, FALSE, 10);
    
    // Server selection
    GtkWidget* serverBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* serverLabel = gtk_label_new("Test Server:");
    gtk_widget_set_size_request(serverLabel, 100, -1);
    gtk_box_pack_start(GTK_BOX(serverBox), serverLabel, FALSE, FALSE, 0);
    
    serverCombo_ = gtk_combo_box_text_new();
    for (const auto& server : servers_) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(serverCombo_), server.name.c_str());
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(serverCombo_), 0);
    gtk_box_pack_start(GTK_BOX(serverBox), serverCombo_, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(container_), serverBox, FALSE, FALSE, 5);
    
    // Progress section
    GtkWidget* progressBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    
    progressBar_ = gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progressBar_), TRUE);
    gtk_box_pack_start(GTK_BOX(progressBox), progressBar_, FALSE, FALSE, 0);
    
    statusLabel_ = gtk_label_new("Ready to test");
    gtk_box_pack_start(GTK_BOX(progressBox), statusLabel_, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(container_), progressBox, FALSE, FALSE, 10);
    
    // Control buttons
    GtkWidget* buttonBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_halign(buttonBox, GTK_ALIGN_CENTER);
    
    startButton_ = gtk_button_new_with_label("Start Test");
    gtk_widget_set_size_request(startButton_, 120, 40);
    g_signal_connect(startButton_, "clicked", G_CALLBACK(onStartClicked), this);
    gtk_box_pack_start(GTK_BOX(buttonBox), startButton_, FALSE, FALSE, 5);
    
    stopButton_ = gtk_button_new_with_label("Stop Test");
    gtk_widget_set_size_request(stopButton_, 120, 40);
    gtk_widget_set_sensitive(stopButton_, FALSE);
    g_signal_connect(stopButton_, "clicked", G_CALLBACK(onStopClicked), this);
    gtk_box_pack_start(GTK_BOX(buttonBox), stopButton_, FALSE, FALSE, 5);
    
    gtk_box_pack_start(GTK_BOX(container_), buttonBox, FALSE, FALSE, 10);
    
    // Results section
    GtkWidget* resultsFrame = gtk_frame_new("Test Results");
    GtkWidget* resultsBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_start(resultsBox, 15);
    gtk_widget_set_margin_end(resultsBox, 15);
    gtk_widget_set_margin_top(resultsBox, 15);
    gtk_widget_set_margin_bottom(resultsBox, 15);
    
    // Download speed
    GtkWidget* downloadBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* downloadTitleLabel = gtk_label_new("Download:");
    gtk_widget_set_size_request(downloadTitleLabel, 100, -1);
    gtk_widget_set_halign(downloadTitleLabel, GTK_ALIGN_START);
    downloadLabel_ = gtk_label_new("--");
    gtk_label_set_markup(GTK_LABEL(downloadLabel_), 
        "<span size='large' weight='bold'>--</span>");
    gtk_widget_set_halign(downloadLabel_, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(downloadBox), downloadTitleLabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(downloadBox), downloadLabel_, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(resultsBox), downloadBox, FALSE, FALSE, 0);
    
    // Upload speed
    GtkWidget* uploadBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* uploadTitleLabel = gtk_label_new("Upload:");
    gtk_widget_set_size_request(uploadTitleLabel, 100, -1);
    gtk_widget_set_halign(uploadTitleLabel, GTK_ALIGN_START);
    uploadLabel_ = gtk_label_new("--");
    gtk_label_set_markup(GTK_LABEL(uploadLabel_), 
        "<span size='large' weight='bold'>--</span>");
    gtk_widget_set_halign(uploadLabel_, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(uploadBox), uploadTitleLabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(uploadBox), uploadLabel_, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(resultsBox), uploadBox, FALSE, FALSE, 0);
    
    // Ping
    GtkWidget* pingBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* pingTitleLabel = gtk_label_new("Ping:");
    gtk_widget_set_size_request(pingTitleLabel, 100, -1);
    gtk_widget_set_halign(pingTitleLabel, GTK_ALIGN_START);
    pingLabel_ = gtk_label_new("--");
    gtk_label_set_markup(GTK_LABEL(pingLabel_), 
        "<span size='large' weight='bold'>--</span>");
    gtk_widget_set_halign(pingLabel_, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(pingBox), pingTitleLabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(pingBox), pingLabel_, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(resultsBox), pingBox, FALSE, FALSE, 0);
    
    // Jitter
    GtkWidget* jitterBox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    GtkWidget* jitterTitleLabel = gtk_label_new("Jitter:");
    gtk_widget_set_size_request(jitterTitleLabel, 100, -1);
    gtk_widget_set_halign(jitterTitleLabel, GTK_ALIGN_START);
    jitterLabel_ = gtk_label_new("--");
    gtk_label_set_markup(GTK_LABEL(jitterLabel_), 
        "<span size='large' weight='bold'>--</span>");
    gtk_widget_set_halign(jitterLabel_, GTK_ALIGN_START);
    gtk_box_pack_start(GTK_BOX(jitterBox), jitterTitleLabel, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(jitterBox), jitterLabel_, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(resultsBox), jitterBox, FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(resultsFrame), resultsBox);
    gtk_box_pack_start(GTK_BOX(container_), resultsFrame, TRUE, TRUE, 10);
    
    gtk_widget_show_all(container_);
    
    return container_;
}

void SpeedTestWidget::onStartClicked(GtkButton* button, gpointer userData) {
    SpeedTestWidget* widget = static_cast<SpeedTestWidget*>(userData);
    widget->startTest();
}

void SpeedTestWidget::onStopClicked(GtkButton* button, gpointer userData) {
    SpeedTestWidget* widget = static_cast<SpeedTestWidget*>(userData);
    widget->stopTest();
}

void SpeedTestWidget::startTest() {
    if (testRunning_) return;
    
    setTestRunning(true);
    
    // Clear previous results
    gtk_label_set_markup(GTK_LABEL(downloadLabel_), "<span size='large' weight='bold'>--</span>");
    gtk_label_set_markup(GTK_LABEL(uploadLabel_), "<span size='large' weight='bold'>--</span>");
    gtk_label_set_markup(GTK_LABEL(pingLabel_), "<span size='large' weight='bold'>--</span>");
    gtk_label_set_markup(GTK_LABEL(jitterLabel_), "<span size='large' weight='bold'>--</span>");
    
    // Run test in separate thread
    std::thread([this]() {
        runTestAsync();
    }).detach();
}

void SpeedTestWidget::stopTest() {
    if (!testRunning_) return;
    
    downloadTest_->stop();
    uploadTest_->stop();
    
    setTestRunning(false);
    gtk_label_set_text(GTK_LABEL(statusLabel_), "Test stopped");
}

void SpeedTestWidget::runTestAsync() {
    int serverIndex = gtk_combo_box_get_active(GTK_COMBO_BOX(serverCombo_));
    if (serverIndex < 0 || serverIndex >= static_cast<int>(servers_.size())) {
        serverIndex = 0;
    }
    
    TestServer server = servers_[serverIndex];
    
    try {
        // Run ping test
        g_idle_add([](gpointer data) -> gboolean {
            SpeedTestWidget* widget = static_cast<SpeedTestWidget*>(data);
            widget->updateProgress("Testing latency...", 0.0, 0.0);
            return G_SOURCE_REMOVE;
        }, this);
        
        PingResults pingResults = pingTest_->run(server.host, 80, 5);
        
        // Run download test
        g_idle_add([](gpointer data) -> gboolean {
            SpeedTestWidget* widget = static_cast<SpeedTestWidget*>(data);
            widget->updateProgress("Testing download speed...", 0.33, 0.0);
            return G_SOURCE_REMOVE;
        }, this);
        
        double downloadSpeed = downloadTest_->run(server.downloadUrl, 4, 10, 2,
            [this](const std::string& stage, double progress, double speed) {
                g_idle_add([](gpointer data) -> gboolean {
                    auto* params = static_cast<std::tuple<SpeedTestWidget*, std::string, double, double>*>(data);
                    std::get<0>(*params)->updateProgress(std::get<1>(*params), 
                                                         0.33 + std::get<2>(*params) * 0.33, 
                                                         std::get<3>(*params));
                    delete params;
                    return G_SOURCE_REMOVE;
                }, new std::tuple<SpeedTestWidget*, std::string, double, double>(this, stage, progress, speed));
            });
        
        // Run upload test
        g_idle_add([](gpointer data) -> gboolean {
            SpeedTestWidget* widget = static_cast<SpeedTestWidget*>(data);
            widget->updateProgress("Testing upload speed...", 0.66, 0.0);
            return G_SOURCE_REMOVE;
        }, this);
        
        double uploadSpeed = uploadTest_->run(server.uploadUrl, 4, 10, 2,
            [this](const std::string& stage, double progress, double speed) {
                g_idle_add([](gpointer data) -> gboolean {
                    auto* params = static_cast<std::tuple<SpeedTestWidget*, std::string, double, double>*>(data);
                    std::get<0>(*params)->updateProgress(std::get<1>(*params), 
                                                         0.66 + std::get<2>(*params) * 0.33, 
                                                         std::get<3>(*params));
                    delete params;
                    return G_SOURCE_REMOVE;
                }, new std::tuple<SpeedTestWidget*, std::string, double, double>(this, stage, progress, speed));
            });
        
        // Update final results
        SpeedTestResult result;
        result.downloadSpeedMbps = downloadSpeed;
        result.uploadSpeedMbps = uploadSpeed;
        result.pingMs = pingResults.avgMs;
        result.jitterMs = pingResults.jitterMs;
        result.success = true;
        
        g_idle_add([](gpointer data) -> gboolean {
            auto* params = static_cast<std::pair<SpeedTestWidget*, SpeedTestResult>*>(data);
            params->first->updateResults(params->second);
            delete params;
            return G_SOURCE_REMOVE;
        }, new std::pair<SpeedTestWidget*, SpeedTestResult>(this, result));
        
    } catch (const std::exception& e) {
        g_idle_add([](gpointer data) -> gboolean {
            auto* params = static_cast<std::pair<SpeedTestWidget*, std::string>*>(data);
            gtk_label_set_text(GTK_LABEL(params->first->statusLabel_), 
                             ("Error: " + params->second).c_str());
            params->first->setTestRunning(false);
            delete params;
            return G_SOURCE_REMOVE;
        }, new std::pair<SpeedTestWidget*, std::string>(this, e.what()));
    }
}

void SpeedTestWidget::updateProgress(const std::string& stage, double progress, double currentSpeed) {
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressBar_), progress);
    
    std::string statusText = stage;
    if (currentSpeed > 0) {
        statusText += " (" + formatSpeed(currentSpeed) + ")";
    }
    gtk_label_set_text(GTK_LABEL(statusLabel_), statusText.c_str());
}

void SpeedTestWidget::updateResults(const SpeedTestResult& result) {
    if (result.success) {
        std::string downloadText = "<span size='large' weight='bold' foreground='#2ecc71'>" + 
                                  formatSpeed(result.downloadSpeedMbps) + "</span>";
        gtk_label_set_markup(GTK_LABEL(downloadLabel_), downloadText.c_str());
        
        std::string uploadText = "<span size='large' weight='bold' foreground='#3498db'>" + 
                                formatSpeed(result.uploadSpeedMbps) + "</span>";
        gtk_label_set_markup(GTK_LABEL(uploadLabel_), uploadText.c_str());
        
        std::string pingText = "<span size='large' weight='bold' foreground='#f39c12'>" + 
                              formatPing(result.pingMs) + "</span>";
        gtk_label_set_markup(GTK_LABEL(pingLabel_), pingText.c_str());
        
        std::string jitterText = "<span size='large' weight='bold'>" + 
                                formatPing(result.jitterMs) + "</span>";
        gtk_label_set_markup(GTK_LABEL(jitterLabel_), jitterText.c_str());
        
        gtk_label_set_text(GTK_LABEL(statusLabel_), "Test completed successfully!");
    }
    
    setTestRunning(false);
}

void SpeedTestWidget::setTestRunning(bool running) {
    testRunning_ = running;
    gtk_widget_set_sensitive(startButton_, !running);
    gtk_widget_set_sensitive(stopButton_, running);
    gtk_widget_set_sensitive(serverCombo_, !running);
    
    if (!running) {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progressBar_), 0.0);
    }
}

std::string SpeedTestWidget::formatSpeed(double mbps) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << mbps << " Mbps";
    return oss.str();
}

std::string SpeedTestWidget::formatPing(double ms) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(1) << ms << " ms";
    return oss.str();
}
