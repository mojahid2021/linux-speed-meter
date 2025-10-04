#include "mainwindow.h"
#include "speed_monitor_qt.h"
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
    QGroupBox* group = new QGroupBox("Data Usage");
    QVBoxLayout* layout = new QVBoxLayout(group);

    // Simple data usage display
    dataUsageLabel_ = new QLabel("Data usage visualization would go here", this);
    dataUsageLabel_->setAlignment(Qt::AlignCenter);
    dataUsageLabel_->setStyleSheet("border: 1px solid #ccc; padding: 20px;");

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

    layout->addWidget(autoStartCheckBox_);
    layout->addWidget(stayOnTopCheckBox_);
    layout->addWidget(notificationsCheckBox_);
    layout->addLayout(refreshLayout);
    layout->addLayout(themeLayout);
    layout->addWidget(resetButton_);
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
    dataUsageLabel_->setText(QString("Total Downloaded: %1\nTotal Uploaded: %2")
        .arg(speedMonitor_->getTotalDownloaded())
        .arg(speedMonitor_->getTotalUploaded()));
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
    QGroupBox* group = new QGroupBox("Current Network Speed");
    QFormLayout* layout = new QFormLayout(group);

    downloadLabel_ = new QLabel("0 B/s", this);
    uploadLabel_ = new QLabel("0 B/s", this);

    // Make labels larger and colored
    QFont speedFont = downloadLabel_->font();
    speedFont.setPointSize(12);
    speedFont.setBold(true);
    downloadLabel_->setFont(speedFont);
    uploadLabel_->setFont(speedFont);

    downloadLabel_->setStyleSheet("color: #4CAF50;");  // Green for download
    uploadLabel_->setStyleSheet("color: #FF9800;");    // Orange for upload

    layout->addRow("↓ Download:", downloadLabel_);
    layout->addRow("↑ Upload:", uploadLabel_);

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
    downloadLabel_->setText(speedMonitor_->getDownloadRate());
    uploadLabel_->setText(speedMonitor_->getUploadRate());

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
