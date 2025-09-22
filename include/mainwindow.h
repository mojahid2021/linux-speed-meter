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
    void createNetworkStatsGroup();
    void createInterfaceInfoGroup();

    SpeedMonitor* speedMonitor_;
    QTimer* updateTimer_;

    // UI elements
    QWidget* centralWidget_;
    QVBoxLayout* mainLayout_;

    // Network stats
    QGroupBox* networkStatsGroup_;
    QLabel* downloadLabel_;
    QLabel* uploadLabel_;
    QLabel* totalLabel_;

    // Interface info
    QGroupBox* interfaceGroup_;
    QLabel* interfaceLabel_;
    QLabel* ipLabel_;
    QLabel* statusLabel_;
};

#endif // MAINWINDOW_H