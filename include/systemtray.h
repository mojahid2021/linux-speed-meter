#ifndef SYSTEMTRAY_H
#define SYSTEMTRAY_H

#include <QSystemTrayIcon>
#include <QMenu>
#include <QTimer>
#include <QIcon>
#include <memory>

class SpeedMonitor;

class SystemTray : public QSystemTrayIcon {
    Q_OBJECT

public:
    explicit SystemTray(SpeedMonitor* monitor, QObject* parent = nullptr);
    ~SystemTray();

    void updateTrayInfo();

signals:
    void showMainWindow();

private slots:
    void onActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowMainWindow();
    void onQuit();

private:
    void createContextMenu();
    void updateIconAndTooltip();

    SpeedMonitor* speedMonitor_;
    QMenu* contextMenu_;
    QTimer* updateTimer_;

    // Icons for different states
    QIcon normalIcon_;
    QIcon activeIcon_;
};

#endif // SYSTEMTRAY_H