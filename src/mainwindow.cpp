#include "mainwindow.h"
#include "speed_monitor_qt.h"
#include "data_exporter.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QProgressBar>
#include <QTimer>
#include <QDateTime>
#include <QTabWidget>
#include <QChartView>
#include <QLineSeries>
#include <QChart>
#include <QValueAxis>
#include <QDateTimeAxis>
#include <QDebug>
#include <QtCharts>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(SpeedMonitor* monitor, QWidget* parent)
    : QMainWindow(parent)
    , speedMonitor_(monitor)
    , updateTimer_(new QTimer(this))
    , centralWidget_(new QWidget(this))
    , tabWidget_(new QTabWidget(this))
    , startTime_(QDateTime::currentDateTime())
    , sessionSeconds_(0)
    , trayIcon_(nullptr)
    , darkMode_(false)
{
    setWindowTitle("Internet Speed Meter - Dashboard");
    setWindowIcon(QIcon(":/icons/network.svg"));
    setMinimumSize(800, 600);

    // Set window to stay on top
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    setupUI();

    // Connect signals
    connect(updateTimer_, &QTimer::timeout, this, &MainWindow::updateDisplay);

    // Start update timer (default 1 second)
    updateTimer_->start(1000);

    // Initial update
    updateDisplay();

    setCentralWidget(centralWidget_);
}

void MainWindow::createSpeedChart(QVBoxLayout* parent) {
    QGroupBox* group = new QGroupBox("Speed Over Time");
    QVBoxLayout* layout = new QVBoxLayout(group);

    // Create chart
    speedChart_ = new QChart();
    speedChart_->setTitle("Network Speed History");

    downloadSeries_ = new QLineSeries();
    downloadSeries_->setName("Download");
    downloadSeries_->setColor(QColor("#4CAF50"));

    uploadSeries_ = new QLineSeries();
    uploadSeries_->setName("Upload");
    uploadSeries_->setColor(QColor("#FF9800"));

    speedChart_->addSeries(downloadSeries_);
    speedChart_->addSeries(uploadSeries_);

    // Create axes
    QDateTimeAxis* axisX = new QDateTimeAxis();
    axisX->setFormat("hh:mm:ss");
    axisX->setTitleText("Time");
    speedChart_->addAxis(axisX, Qt::AlignBottom);

    QValueAxis* axisY = new QValueAxis();
    axisY->setTitleText("Speed (KB/s)");
    axisY->setLabelFormat("%.1f");
    speedChart_->addAxis(axisY, Qt::AlignLeft);

    downloadSeries_->attachAxis(axisX);
    downloadSeries_->attachAxis(axisY);
    uploadSeries_->attachAxis(axisX);
    uploadSeries_->attachAxis(axisY);

    // Create chart view
    QChartView* chartView = new QChartView(speedChart_);
    chartView->setRenderHint(QPainter::Antialiasing);
    layout->addWidget(chartView);

    parent->addWidget(group);
}

void MainWindow::createDataUsageChart(QVBoxLayout* parent) {
    QGroupBox* group = new QGroupBox("💾 Data Usage & Limits");
    group->setStyleSheet("QGroupBox { font-weight: bold; color: #2E86C1; border: 2px solid #BDC3C7; border-radius: 5px; margin-top: 10px; }"
                        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }");
    QVBoxLayout* layout = new QVBoxLayout(group);

    // Data usage progress bar
    QLabel* usageLabel = new QLabel("Data Used:");
    usageLabel->setStyleSheet("font-weight: bold; color: #2E86C1;");
    layout->addWidget(usageLabel);

    dataUsageProgress_ = new QProgressBar();
    dataUsageProgress_->setRange(0, 100);
    dataUsageProgress_->setValue(0);
    dataUsageProgress_->setTextVisible(true);
    dataUsageProgress_->setFixedHeight(25);
    dataUsageProgress_->setStyleSheet("QProgressBar { border: 2px solid #BDC3C7; border-radius: 5px; text-align: center; font-weight: bold; }"
                                     "QProgressBar::chunk { background: qlineargradient(x1:0, y1:0, x2:1, y2:0, stop:0 #3498db, stop:1 #2980b9); }");
    layout->addWidget(dataUsageProgress_);

    // Data limit status with icon
    QHBoxLayout* statusLayout = new QHBoxLayout();
    QLabel* statusIcon = new QLabel("📊");
    statusIcon->setFixedSize(24, 24);
    dataLimitStatusLabel_ = new QLabel("No limit set");
    dataLimitStatusLabel_->setStyleSheet("font-weight: bold; color: #666;");
    statusLayout->addWidget(statusIcon);
    statusLayout->addWidget(dataLimitStatusLabel_);
    statusLayout->addStretch();
    layout->addLayout(statusLayout);

    // Current usage display
    dataUsageLabel_ = new QLabel("Total: 0 B / 0 B");
    dataUsageLabel_->setAlignment(Qt::AlignCenter);
    dataUsageLabel_->setStyleSheet("font-size: 12px; color: #666; background-color: #f8f9fa; padding: 8px; border-radius: 3px; margin-top: 5px;");
    layout->addWidget(dataUsageLabel_);

    parent->addWidget(group);
}

void MainWindow::createSettingsSection(QVBoxLayout* parent) {
    QGroupBox* group = new QGroupBox("Settings");
    QVBoxLayout* layout = new QVBoxLayout(group);

    // Auto-start option
    autoStartCheckBox_ = new QCheckBox("Start automatically on system boot", this);
    autoStartCheckBox_->setChecked(true);

    // Stay on top option
    stayOnTopCheckBox_ = new QCheckBox("Keep window on top", this);
    stayOnTopCheckBox_->setChecked(true);
    connect(stayOnTopCheckBox_, &QCheckBox::toggled, this, &MainWindow::toggleStayOnTop);

    // Notifications option
    notificationsCheckBox_ = new QCheckBox("Enable notifications", this);
    notificationsCheckBox_->setChecked(true);

    // Refresh rate setting
    QHBoxLayout* refreshLayout = new QHBoxLayout();
    QLabel* refreshLabel = new QLabel("Refresh Rate (seconds):", this);
    refreshRateSpinBox_ = new QSpinBox(this);
    refreshRateSpinBox_->setMinimum(1);
    refreshRateSpinBox_->setMaximum(60);
    refreshRateSpinBox_->setValue(1);
    refreshRateSpinBox_->setSuffix(" s");
    connect(refreshRateSpinBox_, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &MainWindow::onRefreshRateChanged);
    refreshLayout->addWidget(refreshLabel);
    refreshLayout->addWidget(refreshRateSpinBox_);
    refreshLayout->addStretch();

    // Theme selection
    QHBoxLayout* themeLayout = new QHBoxLayout();
    QLabel* themeLabel = new QLabel("Theme:", this);
    themeComboBox_ = new QComboBox(this);
    themeComboBox_->addItem("Light");
    themeComboBox_->addItem("Dark");
    connect(themeComboBox_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onThemeChanged);
    themeLayout->addWidget(themeLabel);
    themeLayout->addWidget(themeComboBox_);
    themeLayout->addStretch();

    // Reset statistics button
    resetButton_ = new QPushButton("Reset Statistics", this);
    connect(resetButton_, &QPushButton::clicked, this, &MainWindow::resetStatistics);

    // Export buttons
    QHBoxLayout* exportLayout = new QHBoxLayout();
    exportCSVButton_ = new QPushButton("Export to CSV", this);
    exportJSONButton_ = new QPushButton("Export to JSON", this);
    connect(exportCSVButton_, &QPushButton::clicked, this, &MainWindow::exportToCSV);
    connect(exportJSONButton_, &QPushButton::clicked, this, &MainWindow::exportToJSON);
    exportLayout->addWidget(exportCSVButton_);
    exportLayout->addWidget(exportJSONButton_);

    layout->addWidget(autoStartCheckBox_);
    layout->addWidget(stayOnTopCheckBox_);
    layout->addWidget(notificationsCheckBox_);
    layout->addLayout(refreshLayout);
    layout->addLayout(themeLayout);
    layout->addWidget(resetButton_);
    layout->addLayout(exportLayout);
    layout->addStretch();

    parent->addWidget(group);
}

void MainWindow::updateCharts() {
    // Add current data points to chart
    QDateTime now = QDateTime::currentDateTime();
    double downloadSpeed = parseSpeed(speedMonitor_->getDownloadRate()) / 1024;  // Convert to KB/s
    double uploadSpeed = parseSpeed(speedMonitor_->getUploadRate()) / 1024;

    downloadSeries_->append(now.toMSecsSinceEpoch(), downloadSpeed);
    uploadSeries_->append(now.toMSecsSinceEpoch(), uploadSpeed);

    // Keep only last 60 points (60 seconds)
    while (downloadSeries_->count() > 60) {
        downloadSeries_->remove(0);
    }
    while (uploadSeries_->count() > 60) {
        uploadSeries_->remove(0);
    }

    // Update data usage display
    double totalDownloaded = parseBytes(speedMonitor_->getTotalDownloaded());
    double totalUploaded = parseBytes(speedMonitor_->getTotalUploaded());
    QString usageText = QString("Total: %1 / %2")
        .arg(speedMonitor_->getTotalDownloaded())
        .arg(speedMonitor_->getTotalUploaded());
    dataUsageLabel_->setText(usageText);

    // Update data usage progress bar if limit is set
    // For demo purposes, simulate a 100GB limit
    double dataLimitGB = 100.0;  // 100 GB limit
    double totalDataGB = (totalDownloaded + totalUploaded) / (1024 * 1024 * 1024);
    int usagePercent = std::min(static_cast<int>((totalDataGB / dataLimitGB) * 100), 100);
    dataUsageProgress_->setValue(usagePercent);

    // Update data limit status
    if (dataLimitStatusLabel_) {
        if (usagePercent >= 90) {
            dataLimitStatusLabel_->setText("⚠️ Approaching limit!");
            dataLimitStatusLabel_->setStyleSheet("font-weight: bold; color: #E74C3C;");
        } else if (usagePercent >= 100) {
            dataLimitStatusLabel_->setText("🚫 Limit exceeded!");
            dataLimitStatusLabel_->setStyleSheet("font-weight: bold; color: #E74C3C;");
        } else {
            dataLimitStatusLabel_->setText("✅ Within limit");
            dataLimitStatusLabel_->setStyleSheet("font-weight: bold; color: #28B463;");
        }
    }
}

void MainWindow::toggleStayOnTop(bool stayOnTop) {
    Qt::WindowFlags flags = windowFlags();
    if (stayOnTop) {
        flags |= Qt::WindowStaysOnTopHint;
    } else {
        flags &= ~Qt::WindowStaysOnTopHint;
    }
    setWindowFlags(flags);
    show();  // Need to call show() after changing window flags
}

void MainWindow::resetStatistics() {
    // Reset session data
    startTime_ = QDateTime::currentDateTime();
    sessionSeconds_ = 0;
    peakDownload_ = 0.0;
    peakUpload_ = 0.0;

    // Clear chart data
    downloadSeries_->clear();
    uploadSeries_->clear();

    // Reset labels
    peakDownloadLabel_->setText("0 B/s");
    peakUploadLabel_->setText("0 B/s");
    sessionTimeLabel_->setText("00:00:00");
    totalDataLabel_->setText("0 B / 0 B");
    avgSpeedLabel_->setText("0 B/s / 0 B/s");
}

void MainWindow::setupUI() {
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget_);

    // Title
    QLabel* titleLabel = new QLabel("Internet Usage Dashboard", this);
    titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);

    // Tab widget
    tabWidget_ = new QTabWidget(this);
    mainLayout->addWidget(tabWidget_);

    // Overview tab
    createOverviewTab();

    // Statistics tab
    createStatisticsTab();

    // Charts tab
    createChartsTab();
    
    // Speed Test tab
    createSpeedTestTab();

    // Settings tab
    createSettingsTab();

    // Status bar
    statusBar_ = new QLabel("Ready", this);
    mainLayout->addWidget(statusBar_);
}

void MainWindow::createOverviewTab() {
    QWidget* overviewWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(overviewWidget);

    // Current speeds section
    createCurrentSpeedSection(layout);

    // Session info section
    createSessionInfoSection(layout);

    // Network interface section
    createInterfaceSection(layout);

    tabWidget_->addTab(overviewWidget, "Overview");
}

void MainWindow::createStatisticsTab() {
    QWidget* statsWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(statsWidget);

    // Detailed statistics
    createDetailedStatsSection(layout);

    // Usage summary
    createUsageSummarySection(layout);

    tabWidget_->addTab(statsWidget, "Statistics");
}

void MainWindow::createChartsTab() {
    QWidget* chartsWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(chartsWidget);

    // Speed chart
    createSpeedChart(layout);

    // Data usage chart
    createDataUsageChart(layout);

    tabWidget_->addTab(chartsWidget, "Charts");
}

void MainWindow::createSettingsTab() {
    QWidget* settingsWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(settingsWidget);

    // Settings controls
    createSettingsSection(layout);

    tabWidget_->addTab(settingsWidget, "Settings");
}

void MainWindow::createCurrentSpeedSection(QVBoxLayout* parent) {
    QGroupBox* group = new QGroupBox("📊 Current Network Speed");
    group->setStyleSheet("QGroupBox { font-weight: bold; color: #2E86C1; border: 2px solid #BDC3C7; border-radius: 5px; margin-top: 10px; }"
                        "QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }");
    QVBoxLayout* layout = new QVBoxLayout(group);

    // Download speed with icon and progress bar
    QHBoxLayout* downloadLayout = new QHBoxLayout();
    QLabel* downloadIcon = new QLabel("⬇️");
    downloadIcon->setFixedWidth(30);
    downloadLabel_ = new QLabel("0 B/s");
    downloadLabel_->setStyleSheet("font-size: 14px; font-weight: bold; color: #28B463;");

    downloadProgressBar_ = new QProgressBar();
    downloadProgressBar_->setRange(0, 100);
    downloadProgressBar_->setValue(0);
    downloadProgressBar_->setFixedHeight(20);
    downloadProgressBar_->setStyleSheet("QProgressBar { border: 1px solid #BDC3C7; border-radius: 3px; text-align: center; }"
                                       "QProgressBar::chunk { background-color: #28B463; }");

    downloadLayout->addWidget(downloadIcon);
    downloadLayout->addWidget(downloadLabel_);
    downloadLayout->addWidget(downloadProgressBar_);
    layout->addLayout(downloadLayout);

    // Upload speed with icon and progress bar
    QHBoxLayout* uploadLayout = new QHBoxLayout();
    QLabel* uploadIcon = new QLabel("⬆️");
    uploadIcon->setFixedWidth(30);
    uploadLabel_ = new QLabel("0 B/s");
    uploadLabel_->setStyleSheet("font-size: 14px; font-weight: bold; color: #E67E22;");

    uploadProgressBar_ = new QProgressBar();
    uploadProgressBar_->setRange(0, 100);
    uploadProgressBar_->setValue(0);
    uploadProgressBar_->setFixedHeight(20);
    uploadProgressBar_->setStyleSheet("QProgressBar { border: 1px solid #BDC3C7; border-radius: 3px; text-align: center; }"
                                     "QProgressBar::chunk { background-color: #E67E22; }");

    uploadLayout->addWidget(uploadIcon);
    uploadLayout->addWidget(uploadLabel_);
    uploadLayout->addWidget(uploadProgressBar_);
    layout->addLayout(uploadLayout);

    // Peak speeds display
    QHBoxLayout* peakLayout = new QHBoxLayout();
    QLabel* peakDownloadLabel = new QLabel("Peak ↓: 0 B/s");
    peakDownloadLabel->setStyleSheet("color: #28B463; font-weight: bold;");
    QLabel* peakUploadLabel = new QLabel("Peak ↑: 0 B/s");
    peakUploadLabel->setStyleSheet("color: #E67E22; font-weight: bold;");

    peakLayout->addWidget(peakDownloadLabel);
    peakLayout->addStretch();
    peakLayout->addWidget(peakUploadLabel);
    layout->addLayout(peakLayout);

    parent->addWidget(group);
}

void MainWindow::createSessionInfoSection(QVBoxLayout* parent) {
    QGroupBox* group = new QGroupBox("Session Information");
    QFormLayout* layout = new QFormLayout(group);

    sessionTimeLabel_ = new QLabel("00:00:00", this);
    totalDataLabel_ = new QLabel("0 B / 0 B", this);
    avgSpeedLabel_ = new QLabel("0 B/s / 0 B/s", this);

    layout->addRow("Session Time:", sessionTimeLabel_);
    layout->addRow("Total Data (↓/↑):", totalDataLabel_);
    layout->addRow("Average Speed (↓/↑):", avgSpeedLabel_);

    parent->addWidget(group);
}

void MainWindow::createInterfaceSection(QVBoxLayout* parent) {
    QGroupBox* group = new QGroupBox("Network Interface");
    QFormLayout* layout = new QFormLayout(group);

    interfaceLabel_ = new QLabel("Not detected", this);
    ipLabel_ = new QLabel("Not available", this);
    statusLabel_ = new QLabel("Disconnected", this);

    // Status indicator
    statusIndicator_ = new QLabel("●", this);
    statusIndicator_->setStyleSheet("color: #F44336; font-size: 16px;"); // Red dot

    QHBoxLayout* statusLayout = new QHBoxLayout();
    statusLayout->addWidget(statusIndicator_);
    statusLayout->addWidget(statusLabel_);
    statusLayout->addStretch();

    layout->addRow("Interface:", interfaceLabel_);
    layout->addRow("IP Address:", ipLabel_);
    layout->addRow("Connection Status:", statusLayout);

    parent->addWidget(group);
}

void MainWindow::createInterfaceInfoGroup() {
    interfaceGroup_ = new QGroupBox("Network Interface", this);
    QFormLayout* layout = new QFormLayout(interfaceGroup_);

    interfaceLabel_ = new QLabel("Not detected", this);
    ipLabel_ = new QLabel("Not available", this);
    statusLabel_ = new QLabel("Disconnected", this);

    layout->addRow("Interface:", interfaceLabel_);
    layout->addRow("IP Address:", ipLabel_);
    layout->addRow("Status:", statusLabel_);

    mainLayout_->addWidget(interfaceGroup_);
}

void MainWindow::updateDisplay() {
    if (!speedMonitor_) {
        return;
    }

    sessionSeconds_++;

    // Update current speeds
    QString downloadSpeed = speedMonitor_->getDownloadRate();
    QString uploadSpeed = speedMonitor_->getUploadRate();
    downloadLabel_->setText(downloadSpeed);
    uploadLabel_->setText(uploadSpeed);

    // Update progress bars (convert speeds to percentage for visualization)
    double downloadValue = parseSpeed(speedMonitor_->getDownloadRate());
    double uploadValue = parseSpeed(speedMonitor_->getUploadRate());

    // Scale to 0-100 for progress bar (assuming max 100 MB/s)
    double maxSpeed = 100 * 1024 * 1024; // 100 MB/s
    int downloadPercent = std::min(static_cast<int>((downloadValue / maxSpeed) * 100), 100);
    int uploadPercent = std::min(static_cast<int>((uploadValue / maxSpeed) * 100), 100);

    downloadProgressBar_->setValue(downloadPercent);
    uploadProgressBar_->setValue(uploadPercent);

    // Update session information
    updateSessionInfo();

    // Update interface information
    interfaceLabel_->setText(speedMonitor_->getInterfaceName());
    ipLabel_->setText(speedMonitor_->getIPAddress());

    bool connected = speedMonitor_->isConnected();
    statusLabel_->setText(connected ? "Connected" : "Disconnected");
    statusIndicator_->setStyleSheet(connected ?
        "color: #4CAF50; font-size: 16px;" :  // Green dot
        "color: #F44336; font-size: 16px;");  // Red dot

    // Update detailed statistics
    updateDetailedStats();

    // Update charts
    updateCharts();
    
    // Record usage data for export
    UsageRecord record;
    record.timestamp = QDateTime::currentDateTime();
    record.downloadSpeed = parseSpeed(speedMonitor_->getDownloadRate());
    record.uploadSpeed = parseSpeed(speedMonitor_->getUploadRate());
    record.totalDownload = parseBytes(speedMonitor_->getTotalDownload());
    record.totalUpload = parseBytes(speedMonitor_->getTotalUpload());
    
    usageHistory_.append(record);
    
    // Keep only last 1000 records to avoid excessive memory usage
    if (usageHistory_.size() > 1000) {
        usageHistory_.removeFirst();
    }

#ifdef Q_OS_WIN
    // Update Windows taskbar
    updateWindowsTaskbar();
#endif

    // Update status bar
    statusBar_->setText(QString("Last updated: %1").arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
}

void MainWindow::updateSessionInfo() {
    // Session time
    int hours = sessionSeconds_ / 3600;
    int minutes = (sessionSeconds_ % 3600) / 60;
    int seconds = sessionSeconds_ % 60;
    sessionTimeLabel_->setText(QString("%1:%2:%3")
        .arg(hours, 2, 10, QChar('0'))
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0')));

    // Total data
    totalDataLabel_->setText(QString("%1 / %2")
        .arg(speedMonitor_->getTotalDownloaded())
        .arg(speedMonitor_->getTotalUploaded()));

    // Average speeds
    if (sessionSeconds_ > 0) {
        double avgDownload = parseBytes(speedMonitor_->getTotalDownloaded()) / sessionSeconds_;
        double avgUpload = parseBytes(speedMonitor_->getTotalUploaded()) / sessionSeconds_;
        avgSpeedLabel_->setText(QString("%1 / %2")
            .arg(formatSpeed(avgDownload))
            .arg(formatSpeed(avgUpload)));
    }
}

void MainWindow::updateDetailedStats() {
    // Peak speeds tracking
    double currentDownload = parseSpeed(speedMonitor_->getDownloadRate());
    double currentUpload = parseSpeed(speedMonitor_->getUploadRate());

    if (currentDownload > peakDownload_) {
        peakDownload_ = currentDownload;
        peakDownloadLabel_->setText(formatSpeed(peakDownload_));
    }

    if (currentUpload > peakUpload_) {
        peakUpload_ = currentUpload;
        peakUploadLabel_->setText(formatSpeed(peakUpload_));
    }

    // Total data
    totalDownloadedLabel_->setText(speedMonitor_->getTotalDownloaded());
    totalUploadedLabel_->setText(speedMonitor_->getTotalUploaded());

    // Update progress bars (simulate based on session time)
    int progress = (sessionSeconds_ % 100);  // Cycle every 100 seconds for demo
    downloadProgressBar_->setValue(progress);
    uploadProgressBar_->setValue(progress);
}

double MainWindow::parseSpeed(const QString& speedStr) {
    // Parse speed string like "1.23 MB/s" to bytes per second
    QStringList parts = speedStr.split(" ");
    if (parts.size() >= 2) {
        double value = parts[0].toDouble();
        QString unit = parts[1].toLower();

        if (unit == "mb/s") return value * 1024 * 1024;
        if (unit == "kb/s") return value * 1024;
        if (unit == "b/s") return value;
    }
    return 0.0;
}

double MainWindow::parseBytes(const QString& bytesStr) {
    // Parse bytes string like "1.23 MB" to bytes
    QStringList parts = bytesStr.split(" ");
    if (parts.size() >= 2) {
        double value = parts[0].toDouble();
        QString unit = parts[1].toLower();

        if (unit == "gb") return value * 1024 * 1024 * 1024;
        if (unit == "mb") return value * 1024 * 1024;
        if (unit == "kb") return value * 1024;
        if (unit == "b") return value;
    }
    return 0.0;
}

QString MainWindow::formatSpeed(double bytesPerSecond) {
    if (bytesPerSecond >= 1024 * 1024) {
        return QString("%1 MB/s").arg(bytesPerSecond / (1024 * 1024), 0, 'f', 2);
    } else if (bytesPerSecond >= 1024) {
        return QString("%1 KB/s").arg(bytesPerSecond / 1024, 0, 'f', 2);
    } else {
        return QString("%1 B/s").arg(bytesPerSecond, 0, 'f', 0);
    }
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // Instead of closing, hide the window
    hide();
    event->ignore();
}

void MainWindow::onRefreshRateChanged(int value) {
    // Update timer interval (value is in seconds, convert to milliseconds)
    updateTimer_->setInterval(value * 1000);
}

void MainWindow::onThemeChanged(int index) {
    darkMode_ = (index == 1);
    
    // Apply theme
    QString styleSheet;
    if (darkMode_) {
        styleSheet = R"(
            QMainWindow, QWidget { background-color: #2b2b2b; color: #ffffff; }
            QGroupBox { border: 1px solid #555; border-radius: 5px; margin-top: 10px; padding: 10px; }
            QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px; }
            QLabel { color: #ffffff; }
            QTabWidget::pane { border: 1px solid #555; }
            QTabBar::tab { background: #3c3c3c; color: #ffffff; padding: 8px 16px; }
            QTabBar::tab:selected { background: #4CAF50; }
        )";
    } else {
        styleSheet = "";
    }
    
    qApp->setStyleSheet(styleSheet);
}

void MainWindow::showNotification(const QString& title, const QString& message) {
    if (trayIcon_ && notificationsCheckBox_ && notificationsCheckBox_->isChecked()) {
        trayIcon_->showMessage(title, message, QSystemTrayIcon::Information, 3000);
    }
}

#ifdef Q_OS_WIN
void MainWindow::updateWindowsTaskbar() {
    // Update Windows taskbar tooltip with current speeds
    QString tooltip = QString("↓ %1  ↑ %2")
                        .arg(speedMonitor_->getDownloadRate())
                        .arg(speedMonitor_->getUploadRate());
    setWindowTitle("Speed Meter - " + tooltip);
}
#endif

void MainWindow::exportToCSV() {
    QString filename = QFileDialog::getSaveFileName(this, 
        "Export to CSV", 
        QDir::homePath() + "/speed_meter_data.csv",
        "CSV Files (*.csv)");
    
    if (!filename.isEmpty()) {
        if (DataExporter::exportToCSV(filename, usageHistory_)) {
            QMessageBox::information(this, "Export Successful", 
                QString("Data exported to %1").arg(filename));
        } else {
            QMessageBox::warning(this, "Export Failed", 
                "Failed to export data to CSV file.");
        }
    }
}

void MainWindow::exportToJSON() {
    QString filename = QFileDialog::getSaveFileName(this, 
        "Export to JSON", 
        QDir::homePath() + "/speed_meter_data.json",
        "JSON Files (*.json)");
    
    if (!filename.isEmpty()) {
        if (DataExporter::exportToJSON(filename, usageHistory_)) {
            QMessageBox::information(this, "Export Successful", 
                QString("Data exported to %1").arg(filename));
        } else {
            QMessageBox::warning(this, "Export Failed", 
                "Failed to export data to JSON file.");
        }
    }
}

void MainWindow::createSpeedTestTab() {
    QWidget* speedTestWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(speedTestWidget);
    
    // Create the actual speed test widget
    speedTestWidgetQt_ = new SpeedTestWidgetQt(speedTestWidget);
    layout->addWidget(speedTestWidgetQt_);
    
    tabWidget_->addTab(speedTestWidget, "Speed Test");
}

