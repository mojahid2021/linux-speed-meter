#include "../include/speed_test_widget.h"
#include <thread>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <limits>
#include <chrono>
#include <ctime>
#include <stdexcept>
#include <utility>

SpeedTestWidget::SpeedTestWidget() 
    : container_(nullptr)
    , serverCombo_(nullptr)
    , startButton_(nullptr)
    , stopButton_(nullptr)
    , progressBar_(nullptr)
    , statusLabel_(nullptr)
    , durationSpin_(nullptr)
    , warmupSpin_(nullptr)
    , connectionSpin_(nullptr)
    , autoSelectCheck_(nullptr)
    , historyList_(nullptr)
    , downloadLabel_(nullptr)
    , uploadLabel_(nullptr)
    , pingLabel_(nullptr)
    , jitterLabel_(nullptr)
    , testRunning_(false)
    , cancelRequested_(false)
    , workerActive_(false)
    , aliveToken_(std::make_shared<int>(0)) {
    
    speedTest_ = std::make_unique<SpeedTest>();
    downloadTest_ = std::make_unique<DownloadTest>();
    uploadTest_ = std::make_unique<UploadTest>();
    pingTest_ = std::make_unique<PingTest>();
    
    servers_ = SpeedTest::getDefaultServers();
}

SpeedTestWidget::~SpeedTestWidget() {
    stopTest();
    aliveToken_.reset();
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

    // Advanced settings
    GtkWidget* settingsExpander = gtk_expander_new("Advanced Settings");
    GtkWidget* settingsGrid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(settingsGrid), 6);
    gtk_grid_set_column_spacing(GTK_GRID(settingsGrid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(settingsGrid), 10);

    GtkWidget* durationLabel = gtk_label_new("Duration (s):");
    gtk_widget_set_halign(durationLabel, GTK_ALIGN_START);
    durationSpin_ = gtk_spin_button_new_with_range(5, 120, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(durationSpin_), 15);

    GtkWidget* warmupLabel = gtk_label_new("Warmup (s):");
    gtk_widget_set_halign(warmupLabel, GTK_ALIGN_START);
    warmupSpin_ = gtk_spin_button_new_with_range(0, 30, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(warmupSpin_), 2);

    GtkWidget* connectionLabel = gtk_label_new("Connections:");
    gtk_widget_set_halign(connectionLabel, GTK_ALIGN_START);
    connectionSpin_ = gtk_spin_button_new_with_range(1, 8, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(connectionSpin_), 4);

    autoSelectCheck_ = gtk_check_button_new_with_label("Auto-select fastest server");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autoSelectCheck_), TRUE);

    gtk_grid_attach(GTK_GRID(settingsGrid), durationLabel, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(settingsGrid), durationSpin_, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(settingsGrid), warmupLabel, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(settingsGrid), warmupSpin_, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(settingsGrid), connectionLabel, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(settingsGrid), connectionSpin_, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(settingsGrid), autoSelectCheck_, 0, 3, 2, 1);

    gtk_container_add(GTK_CONTAINER(settingsExpander), settingsGrid);
    gtk_expander_set_expanded(GTK_EXPANDER(settingsExpander), FALSE);
    gtk_box_pack_start(GTK_BOX(container_), settingsExpander, FALSE, FALSE, 5);

    g_signal_connect(durationSpin_, "value-changed", G_CALLBACK(+[](GtkSpinButton*, gpointer userData) {
        static_cast<SpeedTestWidget*>(userData)->handleSettingsChange();
    }), this);
    g_signal_connect(warmupSpin_, "value-changed", G_CALLBACK(+[](GtkSpinButton*, gpointer userData) {
        static_cast<SpeedTestWidget*>(userData)->handleSettingsChange();
    }), this);
    
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

    // History section
    GtkWidget* historyFrame = gtk_frame_new("Recent Results");
    GtkWidget* historyBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(historyBox), 10);
    historyList_ = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(historyList_), GTK_SELECTION_NONE);
    gtk_box_pack_start(GTK_BOX(historyBox), historyList_, TRUE, TRUE, 0);
    gtk_container_add(GTK_CONTAINER(historyFrame), historyBox);
    gtk_box_pack_start(GTK_BOX(container_), historyFrame, TRUE, TRUE, 10);

    refreshHistory();
    
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
    if (testRunning_ || workerActive_.load(std::memory_order_acquire)) {
        return;
    }

    joinWorker();

    handleSettingsChange();

    TestRunConfig config{};
    config.durationSeconds = getDurationSeconds();
    if (config.durationSeconds < 1) {
        config.durationSeconds = 1;
    }
    int warmupValue = getWarmupSeconds();
    int maxWarmup = std::max(0, config.durationSeconds - 1);
    if (warmupValue < 0) {
        warmupValue = 0;
    }
    if (warmupValue > maxWarmup) {
        warmupValue = maxWarmup;
    }
    config.warmupSeconds = warmupValue;
    config.parallelConnections = std::max(1, getParallelConnections());
    config.autoSelect = autoSelectCheck_ &&
                        gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(autoSelectCheck_));
    config.serverIndex = serverCombo_ ?
        gtk_combo_box_get_active(GTK_COMBO_BOX(serverCombo_)) : 0;
    if (config.serverIndex < 0 || config.serverIndex >= static_cast<int>(servers_.size())) {
        config.serverIndex = 0;
    }

    cancelRequested_.store(false, std::memory_order_release);

    setTestRunning(true);

    if (statusLabel_) {
        gtk_label_set_text(GTK_LABEL(statusLabel_), "Preparing test...");
    }

    // Clear previous results
    gtk_label_set_markup(GTK_LABEL(downloadLabel_), "<span size='large' weight='bold'>--</span>");
    gtk_label_set_markup(GTK_LABEL(uploadLabel_), "<span size='large' weight='bold'>--</span>");
    gtk_label_set_markup(GTK_LABEL(pingLabel_), "<span size='large' weight='bold'>--</span>");
    gtk_label_set_markup(GTK_LABEL(jitterLabel_), "<span size='large' weight='bold'>--</span>");

    workerActive_.store(true, std::memory_order_release);
    workerThread_ = std::thread(&SpeedTestWidget::runTestAsync, this, config);
}

void SpeedTestWidget::stopTest() {
    cancelRequested_.store(true, std::memory_order_release);

    downloadTest_->stop();
    uploadTest_->stop();

    if (testRunning_ && statusLabel_) {
        gtk_label_set_text(GTK_LABEL(statusLabel_), "Stopping test...");
    }

    joinWorker();

    if (testRunning_) {
        setTestRunning(false);
        if (statusLabel_) {
            gtk_label_set_text(GTK_LABEL(statusLabel_), "Test cancelled");
        }
    }

    cancelRequested_.store(false, std::memory_order_release);
}

void SpeedTestWidget::runTestAsync(TestRunConfig config) {
    struct WorkerGuard {
        SpeedTestWidget* widget;
        ~WorkerGuard() {
            if (widget) {
                widget->workerActive_.store(false, std::memory_order_release);
            }
        }
    } guard{this};

    auto cancelled = [this]() {
        return isCancelled();
    };

    auto bailIfCancelled = [this, &cancelled]() -> bool {
        if (cancelled()) {
            handleCancellation();
            return true;
        }
        return false;
    };

    if (config.autoSelect) {
        if (bailIfCancelled()) {
            return;
        }
        dispatchToUi([](SpeedTestWidget& widget) {
            widget.updateProgress("Selecting best server...", 0.0, 0.0);
        });
    }

    try {
        auto server = pickServer(config);

        if (bailIfCancelled()) {
            return;
        }

        dispatchToUi([](SpeedTestWidget& widget) {
            widget.updateProgress("Testing latency...", 0.0, 0.0);
        });

        PingResults pingResults = pingTest_->run(server.host, 80, 5);

        if (pingResults.successCount == 0) {
            throw std::runtime_error("Unable to reach the test server. Please check your internet connection or select a different server.");
        }

        if (bailIfCancelled()) {
            return;
        }

        dispatchToUi([](SpeedTestWidget& widget) {
            widget.updateProgress("Testing download speed...", 0.33, 0.0);
        });

        double downloadSpeed = downloadTest_->run(server.downloadUrl, config.parallelConnections,
            config.durationSeconds, config.warmupSeconds,
            [this](const std::string& stage, double progress, double speed) {
                if (isCancelled()) {
                    return;
                }
                auto stageCopy = stage;
                double adjusted = 0.33 + progress * 0.33;
                dispatchToUi([stageCopy, adjusted, speed](SpeedTestWidget& widget) {
                    widget.updateProgress(stageCopy, adjusted, speed);
                });
            });

        if (downloadSpeed == 0.0) {
            throw std::runtime_error("Download test failed to measure any speed. Please check your internet connection.");
        }

        if (bailIfCancelled()) {
            return;
        }

        dispatchToUi([](SpeedTestWidget& widget) {
            widget.updateProgress("Testing upload speed...", 0.66, 0.0);
        });

        double uploadSpeed = uploadTest_->run(server.uploadUrl, config.parallelConnections,
            config.durationSeconds, config.warmupSeconds,
            [this](const std::string& stage, double progress, double speed) {
                if (isCancelled()) {
                    return;
                }
                auto stageCopy = stage;
                double adjusted = 0.66 + progress * 0.33;
                dispatchToUi([stageCopy, adjusted, speed](SpeedTestWidget& widget) {
                    widget.updateProgress(stageCopy, adjusted, speed);
                });
            });

        if (uploadSpeed == 0.0) {
            throw std::runtime_error("Upload test failed to measure any speed. Please check your internet connection.");
        }

        if (bailIfCancelled()) {
            return;
        }

        SpeedTestResult result;
        result.downloadSpeedMbps = downloadSpeed;
        result.uploadSpeedMbps = uploadSpeed;
        result.pingMs = pingResults.avgMs;
        result.jitterMs = pingResults.jitterMs;
        result.success = true;
        result.serverName = server.name;
        result.serverUrl = server.downloadUrl;
        result.timestamp = std::chrono::system_clock::now();

        dispatchToUi([result](SpeedTestWidget& widget) {
            widget.updateResults(result);
        });

    } catch (const std::exception& e) {
        if (cancelled()) {
            handleCancellation();
        } else {
            auto message = std::string("Error: ") + e.what();
            dispatchToUi([message](SpeedTestWidget& widget) {
                if (widget.statusLabel_) {
                    gtk_label_set_text(GTK_LABEL(widget.statusLabel_), message.c_str());
                }
                widget.setTestRunning(false);
            });
        }
    }
}

TestServer SpeedTestWidget::pickServer(const TestRunConfig& config) {
    if (servers_.empty()) {
        throw std::runtime_error("No test servers configured");
    }

    int serverIndex = config.serverIndex;
    if (serverIndex < 0 || serverIndex >= static_cast<int>(servers_.size())) {
        serverIndex = 0;
    }

    if (!config.autoSelect) {
        return servers_[serverIndex];
    }

    double bestPing = std::numeric_limits<double>::infinity();
    int bestIndex = serverIndex;

    for (size_t i = 0; i < servers_.size(); ++i) {
        if (isCancelled()) {
            throw std::runtime_error("Speed test cancelled");
        }

        const auto& candidate = servers_[i];
        PingResults candidatePing = pingTest_->run(candidate.host, 80, 3);
        if (candidatePing.successCount > 0 && candidatePing.avgMs > 0.0 &&
            candidatePing.avgMs < bestPing) {
            bestPing = candidatePing.avgMs;
            bestIndex = static_cast<int>(i);
        }
    }

    if (bestIndex != serverIndex) {
        dispatchToUi([bestIndex](SpeedTestWidget& widget) {
            if (widget.serverCombo_) {
                gtk_combo_box_set_active(GTK_COMBO_BOX(widget.serverCombo_), bestIndex);
            }
        });
    }

    return servers_[bestIndex];
}

void SpeedTestWidget::handleSettingsChange() {
    if (!durationSpin_ || !warmupSpin_) {
        return;
    }

    int duration = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(durationSpin_));
    if (duration < 1) {
        duration = 1;
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(durationSpin_), duration);
    }

    int warmup = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(warmupSpin_));
    if (warmup >= duration) {
        warmup = std::max(0, duration - 1);
        gtk_spin_button_set_value(GTK_SPIN_BUTTON(warmupSpin_), warmup);
    }
}

int SpeedTestWidget::getDurationSeconds() const {
    return durationSpin_ ? gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(durationSpin_)) : 10;
}

int SpeedTestWidget::getWarmupSeconds() const {
    return warmupSpin_ ? gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(warmupSpin_)) : 2;
}

int SpeedTestWidget::getParallelConnections() const {
    return connectionSpin_ ? gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(connectionSpin_)) : 4;
}

void SpeedTestWidget::refreshHistory() {
    if (!historyList_) {
        return;
    }

    GList* children = gtk_container_get_children(GTK_CONTAINER(historyList_));
    for (GList* child = children; child != nullptr; child = child->next) {
        gtk_widget_destroy(GTK_WIDGET(child->data));
    }
    g_list_free(children);

    if (history_.empty()) {
        GtkWidget* placeholder = gtk_label_new("No tests run yet.");
        gtk_widget_set_halign(placeholder, GTK_ALIGN_START);
        gtk_list_box_insert(GTK_LIST_BOX(historyList_), placeholder, -1);
        gtk_widget_show(placeholder);
        return;
    }

    for (const auto& entry : history_) {
        GtkWidget* row = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);

        std::time_t tt = std::chrono::system_clock::to_time_t(entry.timestamp);
        std::tm tm = *std::localtime(&tt);
        std::ostringstream header;
        header << std::put_time(&tm, "%H:%M:%S") << " - " << entry.serverName;

        GtkWidget* headerLabel = gtk_label_new(header.str().c_str());
        gtk_widget_set_halign(headerLabel, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(row), headerLabel, FALSE, FALSE, 0);

        std::ostringstream details;
        details << formatSpeed(entry.downloadSpeedMbps) << " down, "
                << formatSpeed(entry.uploadSpeedMbps) << " up, "
                << formatPing(entry.pingMs) << " ping";
        if (entry.jitterMs > 0.0) {
            details << ", jitter " << formatPing(entry.jitterMs);
        }

        GtkWidget* detailLabel = gtk_label_new(details.str().c_str());
        gtk_widget_set_halign(detailLabel, GTK_ALIGN_START);
        gtk_box_pack_start(GTK_BOX(row), detailLabel, FALSE, FALSE, 0);

        gtk_list_box_insert(GTK_LIST_BOX(historyList_), row, -1);
        gtk_widget_show_all(row);
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

        history_.push_front(result);
        if (history_.size() > 5) {
            history_.pop_back();
        }
        refreshHistory();
    }
    
    setTestRunning(false);
}

void SpeedTestWidget::setTestRunning(bool running) {
    testRunning_ = running;
    gtk_widget_set_sensitive(startButton_, !running);
    gtk_widget_set_sensitive(stopButton_, running);
    gtk_widget_set_sensitive(serverCombo_, !running);
    if (durationSpin_) gtk_widget_set_sensitive(durationSpin_, !running);
    if (warmupSpin_) gtk_widget_set_sensitive(warmupSpin_, !running);
    if (connectionSpin_) gtk_widget_set_sensitive(connectionSpin_, !running);
    if (autoSelectCheck_) gtk_widget_set_sensitive(autoSelectCheck_, !running);
    
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

void SpeedTestWidget::joinWorker() {
    if (workerThread_.joinable()) {
        workerThread_.join();
    }
    workerActive_.store(false, std::memory_order_release);
}

void SpeedTestWidget::handleCancellation() {
    dispatchToUi([](SpeedTestWidget& widget) {
        widget.setTestRunning(false);
        if (widget.statusLabel_) {
            gtk_label_set_text(GTK_LABEL(widget.statusLabel_), "Test cancelled");
        }
    });
}

void SpeedTestWidget::dispatchToUi(std::function<void(SpeedTestWidget&)> fn) {
    if (!fn || !aliveToken_) {
        return;
    }

    struct UiTask {
        std::weak_ptr<void> guard;
        SpeedTestWidget* widget;
        std::function<void(SpeedTestWidget&)> fn;
    };

    auto* task = new UiTask{aliveToken_, this, std::move(fn)};

    g_idle_add_full(G_PRIORITY_DEFAULT, [](gpointer data) -> gboolean {
        UiTask* task = static_cast<UiTask*>(data);
        if (auto guard = task->guard.lock()) {
            task->fn(*task->widget);
        }
        return G_SOURCE_REMOVE;
    }, task, [](gpointer data) {
        delete static_cast<UiTask*>(data);
    });
}

bool SpeedTestWidget::isCancelled() const {
    return cancelRequested_.load(std::memory_order_acquire);
}
