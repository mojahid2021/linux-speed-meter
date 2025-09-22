#include "speed_monitor_win.h"
#include <QDebug>
#include <QNetworkInterface>
#include <QHostAddress>
#include <QDateTime>

SpeedMonitorWin::SpeedMonitorWin(QObject* parent)
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

SpeedMonitorWin::~SpeedMonitorWin() {
    stop();
}

bool SpeedMonitorWin::initialize() {
    // Get network interface information
    QList<QNetworkInterface> interfaces = QNetworkInterface::allInterfaces();

    for (const QNetworkInterface& iface : interfaces) {
        if (iface.flags().testFlag(QNetworkInterface::IsUp) &&
            !iface.flags().testFlag(QNetworkInterface::IsLoopBack)) {

            // Get IPv4 address
            QList<QNetworkAddressEntry> addresses = iface.addressEntries();
            for (const QNetworkAddressEntry& addr : addresses) {
                if (addr.ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    interfaceName_ = iface.humanReadableName();
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

void SpeedMonitorWin::start() {
    if (running_) {
        return;
    }

    running_ = true;

    monitorThread_ = QThread::create([this]() {
        monitorNetwork();
    });

    monitorThread_->start();
}

void SpeedMonitorWin::stop() {
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

void SpeedMonitorWin::monitorNetwork() {
    MIB_IF_TABLE2* ifTable = nullptr;

    while (running_) {
        // Get interface table
        DWORD dwRetVal = GetIfTable2(&ifTable);
        if (dwRetVal != NO_ERROR) {
            qWarning() << "GetIfTable2 failed:" << dwRetVal;
            Sleep(1000);
            continue;
        }

        quint64 currentDownloaded = 0;
        quint64 currentUploaded = 0;

        // Sum up all active interfaces
        for (ULONG i = 0; i < ifTable->NumEntries; i++) {
            MIB_IF_ROW2* row = &ifTable->Table[i];

            // Skip loopback and inactive interfaces
            if (row->InterfaceAndOperStatusFlags.HardwareInterface ||
                row->OperStatus == IfOperStatusUp) {
                currentDownloaded += row->InOctets;
                currentUploaded += row->OutOctets;
            }
        }

        // Update totals
        QMutexLocker locker(&dataMutex_);
        totalDownloaded_ = currentDownloaded;
        totalUploaded_ = currentUploaded;

        // Calculate rates
        downloadRate_ = currentDownloaded - prevDownloaded_;
        uploadRate_ = currentUploaded - prevUploaded_;

        prevDownloaded_ = currentDownloaded;
        prevUploaded_ = currentUploaded;

        // Free the table
        FreeMibTable(ifTable);
        ifTable = nullptr;

        // Sleep for 1 second
        Sleep(1000);
    }

    if (ifTable) {
        FreeMibTable(ifTable);
    }
}

QString SpeedMonitorWin::getLabel() const {
    QString download = formatSpeed(downloadRate_);
    QString upload = formatSpeed(uploadRate_);
    return QString("↓ %1 ↑ %2").arg(download, upload);
}

QString SpeedMonitorWin::getTooltip() const {
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

QString SpeedMonitorWin::getDownloadRate() const {
    return formatSpeed(downloadRate_) + "/s";
}

QString SpeedMonitorWin::getUploadRate() const {
    return formatSpeed(uploadRate_) + "/s";
}

QString SpeedMonitorWin::getTotalDownloaded() const {
    return formatBytes(totalDownloaded_);
}

QString SpeedMonitorWin::getTotalUploaded() const {
    return formatBytes(totalUploaded_);
}

QString SpeedMonitorWin::getInterfaceName() const {
    return interfaceName_;
}

QString SpeedMonitorWin::getIPAddress() const {
    return ipAddress_;
}

bool SpeedMonitorWin::isConnected() const {
    return connected_;
}

bool SpeedMonitorWin::isActive() const {
    return (downloadRate_ > 0 || uploadRate_ > 0);
}

QString SpeedMonitorWin::formatBytes(quint64 bytes) const {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unitIndex = 0;
    double size = bytes;

    while (size >= 1024.0 && unitIndex < 4) {
        size /= 1024.0;
        unitIndex++;
    }

    return QString("%1 %2").arg(size, 0, 'f', 1).arg(units[unitIndex]);
}

QString SpeedMonitorWin::formatSpeed(quint64 bytesPerSecond) const {
    return formatBytes(bytesPerSecond);
}