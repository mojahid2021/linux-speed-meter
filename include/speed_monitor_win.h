#ifndef SPEED_MONITOR_WIN_H
#define SPEED_MONITOR_WIN_H

#include "speed_monitor_qt.h"
#include <windows.h>
#include <iphlpapi.h>
#include <QObject>
#include <QThread>
#include <QMutex>
#include <atomic>

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")

class SpeedMonitorWin : public SpeedMonitor {
    Q_OBJECT

public:
    SpeedMonitorWin(QObject* parent = nullptr);
    ~SpeedMonitorWin();

    bool initialize() override;
    void start() override;
    void stop() override;

    QString getLabel() const override;
    QString getTooltip() const override;
    QString getDownloadRate() const override;
    QString getUploadRate() const override;
    QString getTotalDownloaded() const override;
    QString getTotalUploaded() const override;
    QString getInterfaceName() const override;
    QString getIPAddress() const override;
    bool isConnected() const override;
    bool isActive() const override;

private:
    void monitorNetwork();
    QString formatBytes(quint64 bytes) const;
    QString formatSpeed(quint64 bytesPerSecond) const;

    QThread* monitorThread_;
    mutable QMutex dataMutex_;

    // Network statistics
    std::atomic<quint64> totalDownloaded_;
    std::atomic<quint64> totalUploaded_;
    std::atomic<quint64> downloadRate_;
    std::atomic<quint64> uploadRate_;

    // Previous values for rate calculation
    quint64 prevDownloaded_;
    quint64 prevUploaded_;

    // Interface information
    QString interfaceName_;
    QString ipAddress_;
    bool connected_;

    std::atomic<bool> running_;
};

#endif // SPEED_MONITOR_WIN_H