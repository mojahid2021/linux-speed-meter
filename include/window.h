#ifndef WINDOW_H
#define WINDOW_H
#include <gtk/gtk.h>
#include <string>
#include <chrono>

class Window {
public:
    Window();
    ~Window();
    void show();
    void updateSpeeds(double uploadSpeed, double downloadSpeed, double totalUpload, double totalDownload,
                     const std::string& interface, const std::string& ip, bool connected);
    void handleClose();
    void resetStatistics();

private:
    void createSpeedSection(GtkWidget* parent);
    void createSessionStatsSection(GtkWidget* parent);
    void createInterfaceSection(GtkWidget* parent);
    void createButtonSection(GtkWidget* parent);
    void updateSessionInfo(double totalUpload, double totalDownload);
    void formatSpeed(std::stringstream& ss, double speed, const std::string& prefix);
    std::string formatSpeedSimple(double speed);
    std::string formatBytes(double bytes);

    GtkWidget* window;
    GtkLabel* downloadLabel;
    GtkLabel* uploadLabel;
    GtkLabel* totalLabel;
    GtkLabel* sessionTimeLabel;
    GtkLabel* avgSpeedLabel;
    GtkLabel* interfaceLabel;
    GtkLabel* ipLabel;
    GtkLabel* statusLabel;

    std::chrono::system_clock::time_point startTime;
};

#endif // WINDOW_H
