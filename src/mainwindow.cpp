#include "mainwindow.h"
#include "speed_monitor_qt.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFormLayout>
#include <QDebug>

MainWindow::MainWindow(SpeedMonitor* monitor, QWidget* parent)
    : QMainWindow(parent)
    , speedMonitor_(monitor)
    , updateTimer_(new QTimer(this))
    , centralWidget_(new QWidget(this))
    , mainLayout_(new QVBoxLayout(centralWidget_))
{
    setWindowTitle("Linux Speed Meter");
    setWindowIcon(QIcon(":/icons/app.png"));
    setMinimumSize(400, 300);

    // Set window to stay on top
    setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);

    setupUI();

    // Connect signals
    connect(updateTimer_, &QTimer::timeout, this, &MainWindow::updateDisplay);

    // Start update timer
    updateTimer_->start(1000);

    // Initial update
    updateDisplay();

    setCentralWidget(centralWidget_);
}

MainWindow::~MainWindow() {
    if (updateTimer_) {
        updateTimer_->stop();
    }
}

void MainWindow::setupUI() {
    createNetworkStatsGroup();
    createInterfaceInfoGroup();

    // Add some spacing
    mainLayout_->addStretch();
}

void MainWindow::createNetworkStatsGroup() {
    networkStatsGroup_ = new QGroupBox("Network Statistics", this);
    QFormLayout* layout = new QFormLayout(networkStatsGroup_);

    downloadLabel_ = new QLabel("0 B/s", this);
    uploadLabel_ = new QLabel("0 B/s", this);
    totalLabel_ = new QLabel("Total: 0 B / 0 B", this);

    layout->addRow("Download:", downloadLabel_);
    layout->addRow("Upload:", uploadLabel_);
    layout->addRow("Session:", totalLabel_);

    mainLayout_->addWidget(networkStatsGroup_);
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

    // Update network statistics
    downloadLabel_->setText(speedMonitor_->getDownloadRate());
    uploadLabel_->setText(speedMonitor_->getUploadRate());
    totalLabel_->setText(QString("Total: %1 / %2")
                        .arg(speedMonitor_->getTotalDownloaded())
                        .arg(speedMonitor_->getTotalUploaded()));

    // Update interface information
    interfaceLabel_->setText(speedMonitor_->getInterfaceName());
    ipLabel_->setText(speedMonitor_->getIPAddress());
    statusLabel_->setText(speedMonitor_->isConnected() ? "Connected" : "Disconnected");
}

void MainWindow::closeEvent(QCloseEvent* event) {
    // Instead of closing, hide the window
    hide();
    event->ignore();
}