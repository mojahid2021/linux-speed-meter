#include "data_exporter.h"
#include <QFile>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

bool DataExporter::exportToCSV(const QString& filename, const QVector<UsageRecord>& records) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << formatCSV(records);
    file.close();
    return true;
}

bool DataExporter::exportToJSON(const QString& filename, const QVector<UsageRecord>& records) {
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << formatJSON(records);
    file.close();
    return true;
}

QString DataExporter::formatCSV(const QVector<UsageRecord>& records) {
    QString csv;
    QTextStream out(&csv);
    
    // Header
    out << "Timestamp,Download Speed (MB/s),Upload Speed (MB/s),Total Download (MB),Total Upload (MB)\n";
    
    // Data rows
    for (const auto& record : records) {
        out << record.timestamp.toString(Qt::ISODate) << ","
            << QString::number(record.downloadSpeed / (1024.0 * 1024.0), 'f', 3) << ","
            << QString::number(record.uploadSpeed / (1024.0 * 1024.0), 'f', 3) << ","
            << QString::number(record.totalDownload / (1024.0 * 1024.0), 'f', 2) << ","
            << QString::number(record.totalUpload / (1024.0 * 1024.0), 'f', 2) << "\n";
    }
    
    return csv;
}

QString DataExporter::formatJSON(const QVector<UsageRecord>& records) {
    QJsonObject root;
    QJsonArray dataArray;
    
    for (const auto& record : records) {
        QJsonObject recordObj;
        recordObj["timestamp"] = record.timestamp.toString(Qt::ISODate);
        recordObj["downloadSpeedMBps"] = record.downloadSpeed / (1024.0 * 1024.0);
        recordObj["uploadSpeedMBps"] = record.uploadSpeed / (1024.0 * 1024.0);
        recordObj["totalDownloadMB"] = record.totalDownload / (1024.0 * 1024.0);
        recordObj["totalUploadMB"] = record.totalUpload / (1024.0 * 1024.0);
        dataArray.append(recordObj);
    }
    
    root["exportDate"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    root["recordCount"] = dataArray.size();
    root["data"] = dataArray;
    
    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Indented);
}
