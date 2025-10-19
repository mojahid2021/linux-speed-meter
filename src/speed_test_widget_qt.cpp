#include "../include/speed_test_widget_qt.h"
#include <QGroupBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QFont>
#include <QPalette>

// SpeedTestWorker implementation
SpeedTestWorker::SpeedTestWorker(const TestServer& server)
    : server_(server), stopped_(false) {
    
    downloadTest_ = std::make_unique<DownloadTest>();
    uploadTest_ = std::make_unique<UploadTest>();
    pingTest_ = std::make_unique<PingTest>();
}

SpeedTestWorker::~SpeedTestWorker() {
    stopTest();
}

void SpeedTestWorker::runTest() {
    stopped_ = false;
    SpeedTestResult result;
    result.timestamp = std::chrono::system_clock::now();
    result.serverName = server_.name;
    result.serverUrl = server_.host;
    
    try {
        // Step 1: Ping test
        if (!stopped_) {
            emit progressUpdated("Testing latency...", 0.0, 0.0);
            PingResults pingResults = pingTest_->run(server_.host, 80, 5);
            result.pingMs = pingResults.avgMs;
            result.jitterMs = pingResults.jitterMs;
        }
        
        // Step 2: Download test
        if (!stopped_) {
            emit progressUpdated("Testing download speed...", 0.33, 0.0);
            double downloadSpeed = downloadTest_->run(server_.downloadUrl, 4, 10, 2,
                [this](const std::string& stage, double progress, double speed) {
                    if (!stopped_) {
                        // Download test now reports 0.0-1.0, map to 33%-66% range
                        emit progressUpdated(QString::fromStdString(stage), 
                                           0.33 + progress * 0.33, speed);
                    }
                });
            result.downloadSpeedMbps = downloadSpeed;
        }
        
        // Step 3: Upload test
        if (!stopped_) {
            emit progressUpdated("Testing upload speed...", 0.66, 0.0);
            double uploadSpeed = uploadTest_->run(server_.uploadUrl, 4, 10, 2,
                [this](const std::string& stage, double progress, double speed) {
                    if (!stopped_) {
                        emit progressUpdated(QString::fromStdString(stage), 
                                           0.66 + progress * 0.33, speed);
                    }
                });
            result.uploadSpeedMbps = uploadSpeed;
        }
        
        if (!stopped_) {
            emit progressUpdated("Test complete!", 1.0, 0.0);
            result.success = true;
            emit testCompleted(result);
        }
        
    } catch (const std::exception& e) {
        if (!stopped_) {
            result.success = false;
            result.errorMessage = e.what();
            emit testError(QString::fromStdString(e.what()));
        }
    }
}

void SpeedTestWorker::stopTest() {
    stopped_ = true;
    if (downloadTest_) downloadTest_->stop();
    if (uploadTest_) uploadTest_->stop();
}

// SpeedTestWidgetQt implementation
SpeedTestWidgetQt::SpeedTestWidgetQt(QWidget* parent)
    : QWidget(parent), workerThread_(nullptr), worker_(nullptr), testRunning_(false) {
    
    servers_ = SpeedTest::getDefaultServers();
    setupUI();
}

SpeedTestWidgetQt::~SpeedTestWidgetQt() {
    if (workerThread_) {
        if (worker_) {
            worker_->stopTest();
        }
        workerThread_->quit();
        workerThread_->wait();
        delete workerThread_;
    }
}

void SpeedTestWidgetQt::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    
    // Title with icon
    QHBoxLayout* titleLayout = new QHBoxLayout();
    QLabel* titleIcon = new QLabel("🚀");
    titleIcon->setFixedSize(32, 32);
    QLabel* titleLabel = new QLabel("Internet Speed Test");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #2E86C1;");
    titleLayout->addWidget(titleIcon);
    titleLayout->addWidget(titleLabel);
    titleLayout->addStretch();
    mainLayout->addLayout(titleLayout);
    
    // Server selection group with better styling
    QGroupBox* serverGroup = new QGroupBox("📍 Test Configuration");
    serverGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #2E86C1; border: 2px solid #BDC3C7; border-radius: 5px; margin-top: 10px; }"
                              "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }");
    QFormLayout* serverLayout = new QFormLayout(serverGroup);
    
    serverCombo_ = new QComboBox();
    serverCombo_->setMinimumWidth(300);
    for (const auto& server : servers_) {
        serverCombo_->addItem(QString::fromStdString(server.name));
    }
    serverLayout->addRow("Test Server:", serverCombo_);
    
    mainLayout->addWidget(serverGroup);
    
    // Progress section with better styling
    QGroupBox* progressGroup = new QGroupBox("⚡ Test Progress");
    progressGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #2E86C1; border: 2px solid #BDC3C7; border-radius: 5px; margin-top: 10px; }"
                                "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }");
    QVBoxLayout* progressLayout = new QVBoxLayout(progressGroup);
    
    progressBar_ = new QProgressBar();
    progressBar_->setMinimum(0);
    progressBar_->setMaximum(100);
    progressBar_->setValue(0);
    progressBar_->setTextVisible(true);
    progressBar_->setFixedHeight(25);
    progressBar_->setStyleSheet("QProgressBar { border: 2px solid #BDC3C7; border-radius: 5px; text-align: center; font-weight: bold; }"
                               "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #4CAF50, stop:1 #45a049); }");
    progressLayout->addWidget(progressBar_);
    
    statusLabel_ = new QLabel("Ready to test");
    statusLabel_->setAlignment(Qt::AlignCenter);
    statusLabel_->setStyleSheet("font-weight: bold; color: #666;");
    progressLayout->addWidget(statusLabel_);
    
    mainLayout->addWidget(progressGroup);
    
    // Control buttons with better styling
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    startButton_ = new QPushButton("▶️ Start Test");
    startButton_->setMinimumSize(140, 45);
    startButton_->setStyleSheet("QPushButton { background-color: #4CAF50; color: white; border: none; border-radius: 5px; font-weight: bold; font-size: 12px; }"
                               "QPushButton:hover { background-color: #45a049; }"
                               "QPushButton:pressed { background-color: #3d8b40; }");
    connect(startButton_, &QPushButton::clicked, this, &SpeedTestWidgetQt::onStartClicked);
    buttonLayout->addWidget(startButton_);
    
    stopButton_ = new QPushButton("⏹️ Stop Test");
    stopButton_->setMinimumSize(140, 45);
    stopButton_->setEnabled(false);
    stopButton_->setStyleSheet("QPushButton { background-color: #f44336; color: white; border: none; border-radius: 5px; font-weight: bold; font-size: 12px; }"
                              "QPushButton:hover { background-color: #da190b; }"
                              "QPushButton:pressed { background-color: #b71c1c; }"
                              "QPushButton:disabled { background-color: #cccccc; color: #666666; }");
    connect(stopButton_, &QPushButton::clicked, this, &SpeedTestWidgetQt::onStopClicked);
    buttonLayout->addWidget(stopButton_);
    
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    // Results section with icons and better styling
    QGroupBox* resultsGroup = new QGroupBox("📊 Test Results");
    resultsGroup->setStyleSheet("QGroupBox { font-weight: bold; color: #2E86C1; border: 2px solid #BDC3C7; border-radius: 5px; margin-top: 10px; }"
                               "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }");
    QFormLayout* resultsLayout = new QFormLayout(resultsGroup);
    resultsLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    
    // Download result with icon
    QHBoxLayout* downloadLayout = new QHBoxLayout();
    QLabel* downloadIcon = new QLabel("⬇️");
    downloadIcon->setFixedSize(24, 24);
    downloadLabel_ = new QLabel("--");
    QFont resultFont = downloadLabel_->font();
    resultFont.setPointSize(14);
    resultFont.setBold(true);
    downloadLabel_->setFont(resultFont);
    downloadLabel_->setStyleSheet("color: #28B463; background-color: #f8f9fa; padding: 5px; border-radius: 3px;");
    downloadLayout->addWidget(downloadIcon);
    downloadLayout->addWidget(downloadLabel_);
    downloadLayout->addStretch();
    resultsLayout->addRow("Download:", downloadLayout);
    
    // Upload result with icon
    QHBoxLayout* uploadLayout = new QHBoxLayout();
    QLabel* uploadIcon = new QLabel("⬆️");
    uploadIcon->setFixedSize(24, 24);
    uploadLabel_ = new QLabel("--");
    uploadLabel_->setFont(resultFont);
    uploadLabel_->setStyleSheet("color: #E67E22; background-color: #f8f9fa; padding: 5px; border-radius: 3px;");
    uploadLayout->addWidget(uploadIcon);
    uploadLayout->addWidget(uploadLabel_);
    uploadLayout->addStretch();
    resultsLayout->addRow("Upload:", uploadLayout);
    
    // Ping result with icon
    QHBoxLayout* pingLayout = new QHBoxLayout();
    QLabel* pingIcon = new QLabel("🏓");
    pingIcon->setFixedSize(24, 24);
    pingLabel_ = new QLabel("--");
    pingLabel_->setFont(resultFont);
    pingLabel_->setStyleSheet("color: #9B59B6; background-color: #f8f9fa; padding: 5px; border-radius: 3px;");
    pingLayout->addWidget(pingIcon);
    pingLayout->addWidget(pingLabel_);
    pingLayout->addStretch();
    resultsLayout->addRow("Ping:", pingLayout);
    
    // Jitter result
    jitterLabel_ = new QLabel("--");
    jitterLabel_->setFont(resultFont);
    jitterLabel_->setStyleSheet("color: #34495E; background-color: #f8f9fa; padding: 5px; border-radius: 3px;");
    resultsLayout->addRow("Jitter:", jitterLabel_);
    
    mainLayout->addWidget(resultsGroup);
    
    mainLayout->addStretch();
}

void SpeedTestWidgetQt::onStartClicked() {
    if (testRunning_) return;
    
    setTestRunning(true);
    
    // Clear previous results
    downloadLabel_->setText("--");
    uploadLabel_->setText("--");
    pingLabel_->setText("--");
    jitterLabel_->setText("--");
    
    // Get selected server
    int serverIndex = serverCombo_->currentIndex();
    if (serverIndex < 0 || serverIndex >= static_cast<int>(servers_.size())) {
        serverIndex = 0;
    }
    
    TestServer server = servers_[serverIndex];
    
    // Create worker thread
    workerThread_ = new QThread();
    worker_ = new SpeedTestWorker(server);
    worker_->moveToThread(workerThread_);
    
    // Connect signals
    connect(workerThread_, &QThread::started, worker_, &SpeedTestWorker::runTest);
    connect(worker_, &SpeedTestWorker::progressUpdated, this, &SpeedTestWidgetQt::onProgressUpdated);
    connect(worker_, &SpeedTestWorker::testCompleted, this, &SpeedTestWidgetQt::onTestCompleted);
    connect(worker_, &SpeedTestWorker::testError, this, &SpeedTestWidgetQt::onTestError);
    connect(workerThread_, &QThread::finished, worker_, &QObject::deleteLater);
    connect(workerThread_, &QThread::finished, workerThread_, &QObject::deleteLater);
    
    // Start test
    workerThread_->start();
}

void SpeedTestWidgetQt::onStopClicked() {
    if (!testRunning_) return;
    
    if (worker_) {
        worker_->stopTest();
    }
    
    setTestRunning(false);
    statusLabel_->setText("Test stopped");
}

void SpeedTestWidgetQt::onProgressUpdated(QString stage, double progress, double currentSpeed) {
    progressBar_->setValue(static_cast<int>(progress * 100));
    
    QString statusText = stage;
    if (currentSpeed > 0) {
        statusText += " (" + formatSpeed(currentSpeed) + ")";
    }
    statusLabel_->setText(statusText);
}

void SpeedTestWidgetQt::onTestCompleted(SpeedTestResult result) {
    if (result.success) {
        downloadLabel_->setText(formatSpeed(result.downloadSpeedMbps));
        uploadLabel_->setText(formatSpeed(result.uploadSpeedMbps));
        pingLabel_->setText(formatPing(result.pingMs));
        jitterLabel_->setText(formatPing(result.jitterMs));
        
        statusLabel_->setText("Test completed successfully!");
    }
    
    setTestRunning(false);
    
    // Clean up thread
    if (workerThread_) {
        workerThread_->quit();
        workerThread_->wait();
        workerThread_ = nullptr;
        worker_ = nullptr;
    }
}

void SpeedTestWidgetQt::onTestError(QString error) {
    statusLabel_->setText("Error: " + error);
    setTestRunning(false);
    
    // Clean up thread
    if (workerThread_) {
        workerThread_->quit();
        workerThread_->wait();
        workerThread_ = nullptr;
        worker_ = nullptr;
    }
}

void SpeedTestWidgetQt::setTestRunning(bool running) {
    testRunning_ = running;
    startButton_->setEnabled(!running);
    stopButton_->setEnabled(running);
    serverCombo_->setEnabled(!running);
    
    if (!running) {
        progressBar_->setValue(0);
    }
}

QString SpeedTestWidgetQt::formatSpeed(double mbps) {
    return QString::number(mbps, 'f', 2) + " Mbps";
}

QString SpeedTestWidgetQt::formatPing(double ms) {
    return QString::number(ms, 'f', 1) + " ms";
}
