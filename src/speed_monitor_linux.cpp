#include "speed_monitor_linux.h"
#include "speed_monitor_qt.h"
#include <QNetworkInterface>
#include <QHostAddress>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QMutexLocker>
#include <cmath>

SpeedMonitorLinux::SpeedMonitorLinux(QObject* parent)
    : SpeedMonitor(parent)
    , monitorThread_(nullptr)
    , totalDownloaded_(0)
    , totalUploaded_(0)
    , downloadRate_(0.0)
    , uploadRate_(0.0)
    , prevDownloaded_(0)
    , prevUploaded_(0)
    , timer_()
    , firstSample_(true)
    , smoothedDownload_(0.0)
    , smoothedUpload_(0.0)
    , connected_(false)
    , running_(false)
{
}

SpeedMonitorLinux::~SpeedMonitorLinux() {
    stop();
}

bool SpeedMonitorLinux::initialize() {
    // Get network interface information
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface& iface : interfaces) {
        if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
            !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {

            // Get IPv4 address
            QList<QNetworkAddressEntry> addresses = iface.addressEntries();
            for (const QNetworkAddressEntry& addr : addresses) {
                if (addr.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    interfaceName_ = iface.name();
                    ipAddress_ = addr.ip().toString();
                    connected_ = true;
                    qDebug() << "Using interface:" << interfaceName_ << "IP:" << ipAddress_;
                    return true;
                }
            }
        }
    }

    qWarning() << "No suitable network interface found";
    return false;
}

void SpeedMonitorLinux::start() {
    if (running_.load(std::memory_order_acquire)) {
        return;
    }

    {
        QMutexLocker locker(&dataMutex_);
        totalDownloaded_.store(0, std::memory_order_relaxed);
        totalUploaded_.store(0, std::memory_order_relaxed);
        downloadRate_.store(0.0, std::memory_order_relaxed);
        uploadRate_.store(0.0, std::memory_order_relaxed);
        smoothedDownload_ = 0.0;
        smoothedUpload_ = 0.0;
        prevDownloaded_ = 0;
        prevUploaded_ = 0;
    }
    firstSample_ = true;
    timer_.invalidate();

    running_.store(true, std::memory_order_release);

    monitorThread_ = QThread::create([this]() {
        monitorNetwork();
    });

    monitorThread_->start();
}

void SpeedMonitorLinux::stop() {
    if (!running_.load(std::memory_order_acquire)) {
        return;
    }

    running_.store(false, std::memory_order_release);

    if (monitorThread_) {
        monitorThread_->wait();
        delete monitorThread_;
        monitorThread_ = nullptr;
    }
}

void SpeedMonitorLinux::monitorNetwork() {
    constexpr double kSmoothingAlpha = 0.6;
    constexpr int kPollIntervalMs = 500;

    while (running_.load(std::memory_order_relaxed)) {
        quint64 currentDownloaded = 0;
        quint64 currentUploaded = 0;

        if (readNetworkStats(currentDownloaded, currentUploaded)) {
            if (firstSample_) {
                {
                    QMutexLocker locker(&dataMutex_);
                    totalDownloaded_.store(currentDownloaded, std::memory_order_relaxed);
                    totalUploaded_.store(currentUploaded, std::memory_order_relaxed);
                    prevDownloaded_ = currentDownloaded;
                    prevUploaded_ = currentUploaded;
                    downloadRate_.store(0.0, std::memory_order_relaxed);
                    uploadRate_.store(0.0, std::memory_order_relaxed);
                    smoothedDownload_ = 0.0;
                    smoothedUpload_ = 0.0;
                }
                timer_.start();
                firstSample_ = false;
            } else {
                qint64 elapsedNs = timer_.nsecsElapsed();
                if (elapsedNs <= 0) {
                    elapsedNs = static_cast<qint64>(kPollIntervalMs) * 1000000;
                }
                timer_.restart();

                double elapsedSeconds = static_cast<double>(elapsedNs) / 1'000'000'000.0;
                if (elapsedSeconds <= 1e-6) {
                    elapsedSeconds = static_cast<double>(kPollIntervalMs) / 1000.0;
                }

                quint64 deltaDown = (currentDownloaded >= prevDownloaded_)
                                         ? (currentDownloaded - prevDownloaded_)
                                         : 0;
                quint64 deltaUp = (currentUploaded >= prevUploaded_)
                                       ? (currentUploaded - prevUploaded_)
                                       : 0;

                double instantDown = static_cast<double>(deltaDown) / elapsedSeconds;
                double instantUp = static_cast<double>(deltaUp) / elapsedSeconds;

                double newDownload = kSmoothingAlpha * instantDown +
                                     (1.0 - kSmoothingAlpha) * smoothedDownload_;
                double newUpload = kSmoothingAlpha * instantUp +
                                   (1.0 - kSmoothingAlpha) * smoothedUpload_;

                if (!std::isfinite(newDownload) || newDownload < 0.0) {
                    newDownload = 0.0;
                }
                if (!std::isfinite(newUpload) || newUpload < 0.0) {
                    newUpload = 0.0;
                }

                {
                    QMutexLocker locker(&dataMutex_);
                    totalDownloaded_.store(currentDownloaded, std::memory_order_relaxed);
                    totalUploaded_.store(currentUploaded, std::memory_order_relaxed);
                    smoothedDownload_ = newDownload;
                    smoothedUpload_ = newUpload;
                    downloadRate_.store(smoothedDownload_, std::memory_order_relaxed);
                    uploadRate_.store(smoothedUpload_, std::memory_order_relaxed);
                    prevDownloaded_ = currentDownloaded;
                    prevUploaded_ = currentUploaded;
                }
            }
        }

        QThread::msleep(kPollIntervalMs);
    }
}

bool SpeedMonitorLinux::readNetworkStats(quint64& rx, quint64& tx) {
    if (interfaceName_.isEmpty()) {
        return false;
    }

    QString path = QString("/sys/class/net/%1/statistics/").arg(interfaceName_);
    QFile rxFile(path + "rx_bytes");
    QFile txFile(path + "tx_bytes");

    if (!rxFile.open(QIODevice::ReadOnly) || !txFile.open(QIODevice::ReadOnly)) {
        qWarning() << "Failed to open network statistics files for" << interfaceName_;
        return false;
    }

    QTextStream rxStream(&rxFile);
    QTextStream txStream(&txFile);

    rx = rxStream.readAll().trimmed().toULongLong();
    tx = txStream.readAll().trimmed().toULongLong();

    rxFile.close();
    txFile.close();

    return true;
}

QString SpeedMonitorLinux::getLabel() const {
    QString download = formatSpeed(downloadRate_.load(std::memory_order_relaxed));
    QString upload = formatSpeed(uploadRate_.load(std::memory_order_relaxed));
    return QString("↓ %1 ↑ %2").arg(download, upload);
}

QString SpeedMonitorLinux::getTooltip() const {
    QString download = formatSpeed(downloadRate_.load(std::memory_order_relaxed));
    QString upload = formatSpeed(uploadRate_.load(std::memory_order_relaxed));
    QString totalDown = formatBytes(static_cast<double>(totalDownloaded_.load(std::memory_order_relaxed)));
    QString totalUp = formatBytes(static_cast<double>(totalUploaded_.load(std::memory_order_relaxed)));

    return QString("Linux Speed Meter\n"
                   "Download: %1/s (%2 total)\n"
                   "Upload: %3/s (%4 total)\n"
                   "Interface: %5")
                   .arg(download, totalDown, upload, totalUp, interfaceName_);
}

QString SpeedMonitorLinux::getDownloadRate() const {
    return formatSpeed(downloadRate_.load(std::memory_order_relaxed)) + "/s";
}

QString SpeedMonitorLinux::getUploadRate() const {
    return formatSpeed(uploadRate_.load(std::memory_order_relaxed)) + "/s";
}

QString SpeedMonitorLinux::getTotalDownloaded() const {
    return formatBytes(static_cast<double>(totalDownloaded_.load(std::memory_order_relaxed)));
}

QString SpeedMonitorLinux::getTotalUploaded() const {
    return formatBytes(static_cast<double>(totalUploaded_.load(std::memory_order_relaxed)));
}

QString SpeedMonitorLinux::getInterfaceName() const {
    return interfaceName_;
}

QString SpeedMonitorLinux::getIPAddress() const {
    return ipAddress_;
}

bool SpeedMonitorLinux::isConnected() const {
    return connected_;
}

bool SpeedMonitorLinux::isActive() const {
    return (downloadRate_.load(std::memory_order_relaxed) > 0.0 ||
            uploadRate_.load(std::memory_order_relaxed) > 0.0);
}

QString SpeedMonitorLinux::formatBytes(double bytes) const {
    if (bytes < 0.0) {
        bytes = 0.0;
    }
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = bytes;

    while (size >= 1024.0 && unitIndex < 4) {
        size /= 1024.0;
        unitIndex++;
    }

    int precision = (unitIndex == 0) ? 0 : (size < 10.0 ? 2 : 1);
    return QString("%1 %2").arg(size, 0, 'f', precision).arg(units[unitIndex]);
}

QString SpeedMonitorLinux::formatSpeed(double bytesPerSecond) const {
    if (bytesPerSecond < 0.0) {
        bytesPerSecond = 0.0;
    }
    return formatBytes(bytesPerSecond);
}