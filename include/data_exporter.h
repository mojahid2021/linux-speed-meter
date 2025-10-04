#ifndef DATA_EXPORTER_H
#define DATA_EXPORTER_H

#include <QString>
#include <QVector>
#include <QDateTime>

struct UsageRecord {
    QDateTime timestamp;
    double downloadSpeed;
    double uploadSpeed;
    double totalDownload;
    double totalUpload;
};

class DataExporter {
public:
    static bool exportToCSV(const QString& filename, const QVector<UsageRecord>& records);
    static bool exportToJSON(const QString& filename, const QVector<UsageRecord>& records);
    static QString formatCSV(const QVector<UsageRecord>& records);
    static QString formatJSON(const QVector<UsageRecord>& records);
};

#endif // DATA_EXPORTER_H
