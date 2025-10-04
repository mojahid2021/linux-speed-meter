#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include <QIcon>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>
#include <QCoreApplication>
#include <csignal>
#include <memory>

#include "systemtray.h"
#include "speed_monitor_qt.h"
#include "mainwindow.h"
#include "version.h"

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

void setupAutoStartup() {
#ifdef Q_OS_LINUX
    // For Linux, use the comprehensive auto-startup system
    setupAutoStartupLinux();
#elif defined(Q_OS_WIN)
    // For Windows, use registry-based auto-startup
    setupAutoStartupWindows();
#endif
}

void setupAutoStartupLinux() {
    // Use the same comprehensive Linux auto-startup system as GTK version
    // This will be implemented in a shared utility file
    qDebug() << "Setting up Linux auto-startup...";

    // For now, create a basic XDG autostart entry
    QString autostartDir = QStandardPaths::writableLocation(QStandardPaths::HomeLocation) + "/.config/autostart";
    QDir().mkpath(autostartDir);

    QString desktopFile = autostartDir + "/linux-speed-meter.desktop";
    if (!QFile::exists(desktopFile)) {
        QFile file(desktopFile);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "[Desktop Entry]\n"
                   "Type=Application\n"
                   "Name=Linux Speed Meter\n"
                   "Comment=Network speed monitoring application\n"
                   "Exec=linux-speed-meter\n"
                   "Icon=network-transmit-receive\n"
                   "Categories=Utility;Network;Monitor;\n"
                   "StartupNotify=false\n"
                   "Terminal=false\n"
                   "X-GNOME-Autostart-enabled=true\n"
                   "X-KDE-autostart-after=panel\n"
                   "X-MATE-Autostart-Delay=0\n";
            file.close();
            qDebug() << "Created XDG autostart entry:" << desktopFile;
        }
    }
}

void setupAutoStartupWindows() {
    // Windows registry-based auto-startup
    qDebug() << "Setting up Windows auto-startup...";

#ifdef Q_OS_WIN
    QSettings settings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
                      QSettings::NativeFormat);
    QString appPath = QCoreApplication::applicationFilePath();
    settings.setValue("LinuxSpeedMeter", appPath.replace('/', '\\'));
    qDebug() << "Added to Windows startup:" << appPath;
#endif
}

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    // Set application properties
    app.setApplicationName(APP_NAME);
    app.setApplicationVersion(APP_VERSION);
    app.setOrganizationName(APP_ORGANIZATION);

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

    // Set up auto-startup
    setupAutoStartup();

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