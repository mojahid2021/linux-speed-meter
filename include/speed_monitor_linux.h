#ifndef SPEED_MONITOR_LINUX_H
#define SPEED_MONITOR_LINUX_H

#include "speed_monitor_qt.h"
#include <QObject>
#include <QThread>
#include <QMutex>
#include <QElapsedTimer>
#include <atomic>
#include <fstream>

class SpeedMonitorLinux : public SpeedMonitor {
    Q_OBJECT

public:
    SpeedMonitorLinux(QObject* parent = nullptr);
    ~SpeedMonitorLinux();

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
    QString formatBytes(double bytes) const;
    QString formatSpeed(double bytesPerSecond) const;
    bool readNetworkStats(quint64& rx, quint64& tx);

    QThread* monitorThread_;
    mutable QMutex dataMutex_;

    // Network statistics
    std::atomic<quint64> totalDownloaded_;
    std::atomic<quint64> totalUploaded_;
    std::atomic<double> downloadRate_;
    std::atomic<double> uploadRate_;

    // Previous values for rate calculation
    quint64 prevDownloaded_;
    quint64 prevUploaded_;
    QElapsedTimer timer_;
    bool firstSample_;
    double smoothedDownload_;
    double smoothedUpload_;

    // Interface information
    QString interfaceName_;
    QString ipAddress_;
    bool connected_;

    std::atomic<bool> running_;
};

#endif // SPEED_MONITOR_LINUX_H