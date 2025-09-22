#include "speed_monitor_qt.h"
#include <QNetworkInterface>
#include <QHostAddress>
#include <QDebug>
#include <QFile>
#include <QTextStream>

SpeedMonitorLinux::SpeedMonitorLinux(QObject* parent)
    : SpeedMonitor(parent)
    , monitorThread_(nullptr)
    , totalDownloaded_(0)
    , totalUploaded_(0)
    , downloadRate_(0)
    , uploadRate_(0)
    , prevDownloaded_(0)
    , prevUploaded_(0)
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
    if (running_) {
        return;
    }

    running_ = true;

    monitorThread_ = QThread::create([this]() {
        monitorNetwork();
    });

    monitorThread_->start();
}

void SpeedMonitorLinux::stop() {
    if (!running_) {
        return;
    }

    running_ = false;

    if (monitorThread_) {
        monitorThread_->wait();
        delete monitorThread_;
        monitorThread_ = nullptr;
    }
}

void SpeedMonitorLinux::monitorNetwork() {
    while (running_) {
        quint64 currentDownloaded = 0;
        quint64 currentUploaded = 0;

        if (readNetworkStats(currentDownloaded, currentUploaded)) {
            // Update totals
            QMutexLocker locker(&dataMutex_);
            totalDownloaded_ = currentDownloaded;
            totalUploaded_ = currentUploaded;

            // Calculate rates
            downloadRate_ = currentDownloaded - prevDownloaded_;
            uploadRate_ = currentUploaded - prevUploaded_;

            prevDownloaded_ = currentDownloaded;
            prevUploaded_ = currentUploaded;
        }

        // Sleep for 1 second
        QThread::sleep(1);
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
    QString download = formatSpeed(downloadRate_);
    QString upload = formatSpeed(uploadRate_);
    return QString("↓ %1 ↑ %2").arg(download, upload);
}

QString SpeedMonitorLinux::getTooltip() const {
    QString download = formatSpeed(downloadRate_);
    QString upload = formatSpeed(uploadRate_);
    QString totalDown = formatBytes(totalDownloaded_);
    QString totalUp = formatBytes(totalUploaded_);

    return QString("Linux Speed Meter\n"
                   "Download: %1/s (%2 total)\n"
                   "Upload: %3/s (%4 total)\n"
                   "Interface: %5")
                   .arg(download, totalDown, upload, totalUp, interfaceName_);
}

QString SpeedMonitorLinux::getDownloadRate() const {
    return formatSpeed(downloadRate_) + "/s";
}

QString SpeedMonitorLinux::getUploadRate() const {
    return formatSpeed(uploadRate_) + "/s";
}

QString SpeedMonitorLinux::getTotalDownloaded() const {
    return formatBytes(totalDownloaded_);
}

QString SpeedMonitorLinux::getTotalUploaded() const {
    return formatBytes(totalUploaded_);
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
    return (downloadRate_ > 0 || uploadRate_ > 0);
}

QString SpeedMonitorLinux::formatBytes(quint64 bytes) const {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = bytes;

    while (size >= 1024.0 && unitIndex < 4) {
        size /= 1024.0;
        unitIndex++;
    }

    return QString("%1 %2").arg(size, 0, 'f', 1).arg(units[unitIndex]);
}

QString SpeedMonitorLinux::formatSpeed(quint64 bytesPerSecond) const {
    return formatBytes(bytesPerSecond);
}