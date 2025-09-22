#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include <QIcon>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QMessageBox>
#include <csignal>
#include <memory>

#include "systemtray.h"
#include "speed_monitor_qt.h"
#include "mainwindow.h"

#ifdef Q_OS_WIN
#include "speed_monitor_win.h"
#else
#include "speed_monitor_linux.h"
#endif

static std::atomic<bool> global_running(true);

void signal_handler(int signal) {
    Q_UNUSED(signal)
    global_running = false;
    QApplication::quit();
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName("Linux Speed Meter");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("LinuxSpeedMeter");

    // Install signal handlers
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    // Check if system tray is available
    if (!QSystemTrayIcon::isSystemTrayAvailable()) {
        QMessageBox::critical(nullptr, "System Tray",
            "System tray is not available on this system.\n"
            "The application requires a system tray to function properly.");
        return 1;
    }

    // Create speed monitor
#ifdef Q_OS_WIN
    auto speedMonitor = std::make_unique<SpeedMonitorWin>();
#else
    auto speedMonitor = std::make_unique<SpeedMonitorLinux>();
#endif

    if (!speedMonitor->initialize()) {
        QMessageBox::critical(nullptr, "Initialization Error",
            "Failed to initialize network monitoring.\n"
            "Please check your network interfaces.");
        return 1;
    }

    // Create system tray
    SystemTray trayIcon(speedMonitor.get());

    // Start monitoring
    speedMonitor->start();

    // Create main window (hidden by default)
    MainWindow mainWindow(speedMonitor.get());
    mainWindow.hide();

    // Connect tray icon to show main window
    QObject::connect(&trayIcon, &SystemTray::showMainWindow,
                     &mainWindow, &MainWindow::show);

    // Show tray icon
    trayIcon.show();

    // Start the event loop
    int result = app.exec();

    // Cleanup
    speedMonitor->stop();
    global_running = false;

    return result;
}