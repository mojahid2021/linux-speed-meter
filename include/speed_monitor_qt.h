#ifndef SPEED_MONITOR_QT_H
#define SPEED_MONITOR_QT_H

#include <QObject>
#include <QString>

class SpeedMonitor : public QObject {
    Q_OBJECT

public:
    explicit SpeedMonitor(QObject* parent = nullptr) : QObject(parent) {}
    virtual ~SpeedMonitor() = default;

    // Pure virtual methods that must be implemented by subclasses
    virtual bool initialize() = 0;
    virtual void start() = 0;
    virtual void stop() = 0;

    virtual QString getLabel() const = 0;
    virtual QString getTooltip() const = 0;
    virtual QString getDownloadRate() const = 0;
    virtual QString getUploadRate() const = 0;
    virtual QString getTotalDownloaded() const = 0;
    virtual QString getTotalUploaded() const = 0;
    virtual QString getInterfaceName() const = 0;
    virtual QString getIPAddress() const = 0;
    virtual bool isConnected() const = 0;
    virtual bool isActive() const = 0;

signals:
    void dataUpdated();
    void connectionChanged(bool connected);
};

#endif // SPEED_MONITOR_QT_H