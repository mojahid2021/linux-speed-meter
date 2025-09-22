#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QGroupBox>
#include <QTimer>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include <QTabWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QProgressBar>
#include <QDateTime>
#include <QtCharts>

class SpeedMonitor;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(SpeedMonitor* monitor, QWidget* parent = nullptr);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void updateDisplay();

private:
    void setupUI();

    // Tab creation methods
    void createOverviewTab();
    void createStatisticsTab();
    void createChartsTab();
    void createSettingsTab();

    // Section creation methods
    void createCurrentSpeedSection(QVBoxLayout* parent);
    void createSessionInfoSection(QVBoxLayout* parent);
    void createInterfaceSection(QVBoxLayout* parent);
    void createDetailedStatsSection(QVBoxLayout* parent);
    void createUsageSummarySection(QVBoxLayout* parent);
    void createSpeedChart(QVBoxLayout* parent);
    void createDataUsageChart(QVBoxLayout* parent);
    void createSettingsSection(QVBoxLayout* parent);

    // Update methods
    void updateSessionInfo();
    void updateDetailedStats();
    void updateCharts();

    // Utility methods
    double parseSpeed(const QString& speedStr);
    double parseBytes(const QString& bytesStr);
    QString formatSpeed(double bytesPerSecond);

    // Settings slots
    void toggleStayOnTop(bool stayOnTop);
    void resetStatistics();

    // Core components
    SpeedMonitor* speedMonitor_;
    QTimer* updateTimer_;
    QTabWidget* tabWidget_;

    // Session tracking
    QDateTime startTime_;
    int sessionSeconds_;
    double peakDownload_;
    double peakUpload_;

    // UI elements
    QWidget* centralWidget_;
    QLabel* statusBar_;

    // Overview tab
    QLabel* downloadLabel_;
    QLabel* uploadLabel_;
    QLabel* sessionTimeLabel_;
    QLabel* totalDataLabel_;
    QLabel* avgSpeedLabel_;
    QLabel* interfaceLabel_;
    QLabel* ipLabel_;
    QLabel* statusLabel_;
    QLabel* statusIndicator_;

    // Statistics tab
    QLabel* peakDownloadLabel_;
    QLabel* peakUploadLabel_;
    QLabel* totalDownloadedLabel_;
    QLabel* totalUploadedLabel_;
    QProgressBar* downloadProgressBar_;
    QProgressBar* uploadProgressBar_;

    // Charts tab
    QChart* speedChart_;
    QLineSeries* downloadSeries_;
    QLineSeries* uploadSeries_;
    QLabel* dataUsageLabel_;

    // Settings tab
    QCheckBox* autoStartCheckBox_;
    QCheckBox* stayOnTopCheckBox_;
    QPushButton* resetButton_;
};

#endif // MAINWINDOW_H