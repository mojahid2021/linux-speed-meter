#include "systemtray.h"
#include "speed_monitor_qt.h"
#include <QAction>
#include <QApplication>
#include <QStyle>

SystemTray::SystemTray(SpeedMonitor* monitor, QObject* parent)
    : QSystemTrayIcon(parent)
    , speedMonitor_(monitor)
    , contextMenu_(nullptr)
    , updateTimer_(nullptr)
{
    // Create icons
    normalIcon_ = QIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
    activeIcon_ = QIcon(QApplication::style()->standardIcon(QStyle::SP_DriveNetIcon));
    
    setIcon(normalIcon_);
    
    // Create context menu
    createContextMenu();
    
    // Set up update timer
    updateTimer_ = new QTimer(this);
    connect(updateTimer_, &QTimer::timeout, this, &SystemTray::updateIconAndTooltip);
    updateTimer_->start(1000); // Update every second
    
    // Connect activation signal
    connect(this, &QSystemTrayIcon::activated, this, &SystemTray::onActivated);
}

SystemTray::~SystemTray()
{
    if (updateTimer_) {
        updateTimer_->stop();
    }
}

void SystemTray::createContextMenu()
{
    contextMenu_ = new QMenu();
    
    QAction* showAction = contextMenu_->addAction("Show Dashboard");
    connect(showAction, &QAction::triggered, this, &SystemTray::onShowMainWindow);
    
    contextMenu_->addSeparator();
    
    QAction* quitAction = contextMenu_->addAction("Quit");
    connect(quitAction, &QAction::triggered, this, &SystemTray::onQuit);
    
    setContextMenu(contextMenu_);
}

void SystemTray::updateIconAndTooltip()
{
    if (!speedMonitor_) {
        return;
    }
    
    // Get current speeds
    QString downloadSpeed = speedMonitor_->getDownloadRate();
    QString uploadSpeed = speedMonitor_->getUploadRate();
    
    // Update tooltip
    QString tooltip = QString("Linux Speed Meter\n↓ %1\n↑ %2")
                        .arg(downloadSpeed)
                        .arg(uploadSpeed);
    setToolTip(tooltip);
    
    // Update icon based on activity
    bool isActive = speedMonitor_->isConnected();
    setIcon(isActive ? activeIcon_ : normalIcon_);
}

void SystemTray::updateTrayInfo()
{
    updateIconAndTooltip();
}

void SystemTray::onActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger || reason == QSystemTrayIcon::DoubleClick) {
        emit showMainWindow();
    }
}

void SystemTray::onShowMainWindow()
{
    emit showMainWindow();
}

void SystemTray::onQuit()
{
    QApplication::quit();
}
