#ifndef SPEED_TEST_WIDGET_QT_H
#define SPEED_TEST_WIDGET_QT_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QComboBox>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QThread>
#include <memory>
#include "speed_test.h"
#include "download_test.h"
#include "upload_test.h"
#include "ping_test.h"

// Worker thread for running tests
class SpeedTestWorker : public QObject {
    Q_OBJECT

public:
    SpeedTestWorker(const TestServer& server);
    ~SpeedTestWorker();

public slots:
    void runTest();
    void stopTest();

signals:
    void progressUpdated(QString stage, double progress, double currentSpeed);
    void testCompleted(SpeedTestResult result);
    void testError(QString error);

private:
    TestServer server_;
    std::unique_ptr<DownloadTest> downloadTest_;
    std::unique_ptr<UploadTest> uploadTest_;
    std::unique_ptr<PingTest> pingTest_;
    bool stopped_;
};

// Qt widget for speed test interface
class SpeedTestWidgetQt : public QWidget {
    Q_OBJECT

public:
    explicit SpeedTestWidgetQt(QWidget* parent = nullptr);
    ~SpeedTestWidgetQt();

private slots:
    void onStartClicked();
    void onStopClicked();
    void onProgressUpdated(QString stage, double progress, double currentSpeed);
    void onTestCompleted(SpeedTestResult result);
    void onTestError(QString error);

private:
    void setupUI();
    void setTestRunning(bool running);
    QString formatSpeed(double mbps);
    QString formatPing(double ms);

    // UI Components
    QComboBox* serverCombo_;
    QPushButton* startButton_;
    QPushButton* stopButton_;
    QProgressBar* progressBar_;
    QLabel* statusLabel_;
    
    // Result labels
    QLabel* downloadLabel_;
    QLabel* uploadLabel_;
    QLabel* pingLabel_;
    QLabel* jitterLabel_;
    
    // Test data
    std::vector<TestServer> servers_;
    QThread* workerThread_;
    SpeedTestWorker* worker_;
    bool testRunning_;
};

#endif // SPEED_TEST_WIDGET_QT_H
