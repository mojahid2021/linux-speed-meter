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
    
    // Title
    QLabel* titleLabel = new QLabel("Internet Speed Test");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);
    
    // Server selection group
    QGroupBox* serverGroup = new QGroupBox("Test Configuration");
    QFormLayout* serverLayout = new QFormLayout(serverGroup);
    
    serverCombo_ = new QComboBox();
    for (const auto& server : servers_) {
        serverCombo_->addItem(QString::fromStdString(server.name));
    }
    serverLayout->addRow("Test Server:", serverCombo_);
    
    mainLayout->addWidget(serverGroup);
    
    // Progress section
    QGroupBox* progressGroup = new QGroupBox("Test Progress");
    QVBoxLayout* progressLayout = new QVBoxLayout(progressGroup);
    
    progressBar_ = new QProgressBar();
    progressBar_->setMinimum(0);
    progressBar_->setMaximum(100);
    progressBar_->setValue(0);
    progressBar_->setTextVisible(true);
    progressLayout->addWidget(progressBar_);
    
    statusLabel_ = new QLabel("Ready to test");
    statusLabel_->setAlignment(Qt::AlignCenter);
    progressLayout->addWidget(statusLabel_);
    
    mainLayout->addWidget(progressGroup);
    
    // Control buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch();
    
    startButton_ = new QPushButton("Start Test");
    startButton_->setMinimumSize(120, 40);
    connect(startButton_, &QPushButton::clicked, this, &SpeedTestWidgetQt::onStartClicked);
    buttonLayout->addWidget(startButton_);
    
    stopButton_ = new QPushButton("Stop Test");
    stopButton_->setMinimumSize(120, 40);
    stopButton_->setEnabled(false);
    connect(stopButton_, &QPushButton::clicked, this, &SpeedTestWidgetQt::onStopClicked);
    buttonLayout->addWidget(stopButton_);
    
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);
    
    // Results section
    QGroupBox* resultsGroup = new QGroupBox("Test Results");
    QFormLayout* resultsLayout = new QFormLayout(resultsGroup);
    resultsLayout->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    
    downloadLabel_ = new QLabel("--");
    QFont resultFont = downloadLabel_->font();
    resultFont.setPointSize(14);
    resultFont.setBold(true);
    downloadLabel_->setFont(resultFont);
    downloadLabel_->setStyleSheet("color: #4CAF50;");
    resultsLayout->addRow("Download:", downloadLabel_);
    
    uploadLabel_ = new QLabel("--");
    uploadLabel_->setFont(resultFont);
    uploadLabel_->setStyleSheet("color: #2196F3;");
    resultsLayout->addRow("Upload:", uploadLabel_);
    
    pingLabel_ = new QLabel("--");
    pingLabel_->setFont(resultFont);
    pingLabel_->setStyleSheet("color: #FF9800;");
    resultsLayout->addRow("Ping:", pingLabel_);
    
    jitterLabel_ = new QLabel("--");
    jitterLabel_->setFont(resultFont);
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
