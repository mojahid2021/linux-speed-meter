#include "../include/window.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>

Window::Window() : window(nullptr), uploadLabel(nullptr), downloadLabel(nullptr),
                   totalLabel(nullptr), interfaceLabel(nullptr), ipLabel(nullptr),
                   statusLabel(nullptr), startTime(std::chrono::system_clock::now()) {}

Window::~Window() {
    if (window) {
        gtk_widget_destroy(window);
    }
}

void Window::show() {
    if (!window) {
        // Initialize the GTK window
        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), "Internet Speed Meter - Dashboard");
        gtk_window_set_default_size(GTK_WINDOW(window), 500, 400);

        // Create main vertical box
        GtkWidget* mainVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_container_add(GTK_CONTAINER(window), mainVBox);
        gtk_container_set_border_width(GTK_CONTAINER(mainVBox), 10);

        // Title
        GtkWidget* titleLabel = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(titleLabel), "<span size='large' weight='bold'>Internet Usage Dashboard</span>");
        gtk_box_pack_start(GTK_BOX(mainVBox), titleLabel, FALSE, FALSE, 5);

        // Current speeds section
        createSpeedSection(mainVBox);

        // Session statistics section
        createSessionStatsSection(mainVBox);

        // Network interface section
        createInterfaceSection(mainVBox);

        // Control buttons
        createButtonSection(mainVBox);

        // Connect the close event
        g_signal_connect(window, "destroy", G_CALLBACK(+[](GtkWidget*, gpointer self) {
            static_cast<Window*>(self)->handleClose();
        }), this);

        gtk_widget_show_all(window);
    } else {
        gtk_widget_show(window);
    }
}

void Window::createSpeedSection(GtkWidget* parent) {
    GtkWidget* frame = gtk_frame_new("Current Speed");
    gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 5);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(frame), vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    // Download speed
    downloadLabel = GTK_LABEL(gtk_label_new("Download: 0 B/s"));
    gtk_label_set_xalign(GTK_LABEL(downloadLabel), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(downloadLabel), FALSE, FALSE, 2);

    // Upload speed
    uploadLabel = GTK_LABEL(gtk_label_new("Upload: 0 B/s"));
    gtk_label_set_xalign(GTK_LABEL(uploadLabel), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(uploadLabel), FALSE, FALSE, 2);
}

void Window::createSessionStatsSection(GtkWidget* parent) {
    GtkWidget* frame = gtk_frame_new("Session Statistics");
    gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 5);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(frame), vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    // Total data transferred
    totalLabel = GTK_LABEL(gtk_label_new("Total: 0 B downloaded, 0 B uploaded"));
    gtk_label_set_xalign(GTK_LABEL(totalLabel), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(totalLabel), FALSE, FALSE, 2);

    // Session time
    sessionTimeLabel = GTK_LABEL(gtk_label_new("Session Time: 00:00:00"));
    gtk_label_set_xalign(GTK_LABEL(sessionTimeLabel), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(sessionTimeLabel), FALSE, FALSE, 2);

    // Average speeds
    avgSpeedLabel = GTK_LABEL(gtk_label_new("Average: 0 B/s down, 0 B/s up"));
    gtk_label_set_xalign(GTK_LABEL(avgSpeedLabel), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(avgSpeedLabel), FALSE, FALSE, 2);
}

void Window::createInterfaceSection(GtkWidget* parent) {
    GtkWidget* frame = gtk_frame_new("Network Interface");
    gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 5);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(frame), vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    // Interface name
    interfaceLabel = GTK_LABEL(gtk_label_new("Interface: Not detected"));
    gtk_label_set_xalign(GTK_LABEL(interfaceLabel), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(interfaceLabel), FALSE, FALSE, 2);

    // IP Address
    ipLabel = GTK_LABEL(gtk_label_new("IP Address: Not available"));
    gtk_label_set_xalign(GTK_LABEL(ipLabel), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(ipLabel), FALSE, FALSE, 2);

    // Connection status
    statusLabel = GTK_LABEL(gtk_label_new("Status: Disconnected"));
    gtk_label_set_xalign(GTK_LABEL(statusLabel), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(statusLabel), FALSE, FALSE, 2);
}

void Window::createButtonSection(GtkWidget* parent) {
    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(parent), hbox, FALSE, FALSE, 5);

    // Reset button
    GtkWidget* resetButton = gtk_button_new_with_label("Reset Statistics");
    g_signal_connect(resetButton, "clicked", G_CALLBACK(+[](GtkWidget*, gpointer self) {
        static_cast<Window*>(self)->resetStatistics();
    }), this);
    gtk_box_pack_end(GTK_BOX(hbox), resetButton, FALSE, FALSE, 0);

    // Close button
    GtkWidget* closeButton = gtk_button_new_with_label("Close");
    g_signal_connect(closeButton, "clicked", G_CALLBACK(+[](GtkWidget*, gpointer self) {
        static_cast<Window*>(self)->handleClose();
    }), this);
    gtk_box_pack_end(GTK_BOX(hbox), closeButton, FALSE, FALSE, 0);
}

void Window::updateSpeeds(double uploadSpeed, double downloadSpeed, double totalUpload, double totalDownload,
                         const std::string& interface, const std::string& ip, bool connected) {
    if (downloadLabel && uploadLabel) {
        // Update current speeds
        std::stringstream downloadText, uploadText;
        formatSpeed(downloadText, downloadSpeed, "Download: ");
        formatSpeed(uploadText, uploadSpeed, "Upload: ");
        gtk_label_set_text(downloadLabel, downloadText.str().c_str());
        gtk_label_set_text(uploadLabel, uploadText.str().c_str());
    }

    if (totalLabel) {
        // Update total statistics
        std::stringstream totalText;
        totalText << "Total: " << formatBytes(totalDownload) << " downloaded, "
                  << formatBytes(totalUpload) << " uploaded";
        gtk_label_set_text(totalLabel, totalText.str().c_str());
    }

    if (interfaceLabel) {
        std::string interfaceText = "Interface: " + (interface.empty() ? "Not detected" : interface);
        gtk_label_set_text(interfaceLabel, interfaceText.c_str());
    }

    if (ipLabel) {
        std::string ipText = "IP Address: " + (ip.empty() ? "Not available" : ip);
        gtk_label_set_text(ipLabel, ipText.c_str());
    }

    if (statusLabel) {
        std::string statusText = "Status: " + std::string(connected ? "Connected" : "Disconnected");
        gtk_label_set_text(statusLabel, statusText.c_str());
    }

    // Update session time and averages
    updateSessionInfo(totalUpload, totalDownload);
}

void Window::updateSessionInfo(double totalUpload, double totalDownload) {
    if (!sessionTimeLabel || !avgSpeedLabel) return;

    // Calculate session time
    auto now = std::chrono::system_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - startTime);
    auto hours = duration.count() / 3600;
    auto minutes = (duration.count() % 3600) / 60;
    auto seconds = duration.count() % 60;

    std::stringstream timeText;
    timeText << "Session Time: "
             << std::setfill('0') << std::setw(2) << hours << ":"
             << std::setfill('0') << std::setw(2) << minutes << ":"
             << std::setfill('0') << std::setw(2) << seconds;
    gtk_label_set_text(sessionTimeLabel, timeText.str().c_str());

    // Calculate average speeds
    if (duration.count() > 0) {
        double avgDownload = totalDownload / duration.count();
        double avgUpload = totalUpload / duration.count();

        std::stringstream avgText;
        avgText << "Average: " << formatSpeedSimple(avgDownload) << " down, "
                << formatSpeedSimple(avgUpload) << " up";
        gtk_label_set_text(avgSpeedLabel, avgText.str().c_str());
    }
}

void Window::formatSpeed(std::stringstream& ss, double speed, const std::string& prefix) {
    ss << prefix;
    if (speed >= 1024 * 1024) {
        ss << std::fixed << std::setprecision(2) << (speed / (1024 * 1024)) << " MB/s";
    } else if (speed >= 1024) {
        ss << std::fixed << std::setprecision(2) << (speed / 1024) << " KB/s";
    } else {
        ss << std::fixed << std::setprecision(0) << speed << " B/s";
    }
}

std::string Window::formatSpeedSimple(double speed) {
    std::stringstream ss;
    if (speed >= 1024 * 1024) {
        ss << std::fixed << std::setprecision(2) << (speed / (1024 * 1024)) << " MB/s";
    } else if (speed >= 1024) {
        ss << std::fixed << std::setprecision(2) << (speed / 1024) << " KB/s";
    } else {
        ss << std::fixed << std::setprecision(0) << speed << " B/s";
    }
    return ss.str();
}

std::string Window::formatBytes(double bytes) {
    std::stringstream ss;
    if (bytes >= 1024 * 1024 * 1024) {
        ss << std::fixed << std::setprecision(2) << (bytes / (1024 * 1024 * 1024)) << " GB";
    } else if (bytes >= 1024 * 1024) {
        ss << std::fixed << std::setprecision(2) << (bytes / (1024 * 1024)) << " MB";
    } else if (bytes >= 1024) {
        ss << std::fixed << std::setprecision(2) << (bytes / 1024) << " KB";
    } else {
        ss << std::fixed << std::setprecision(0) << bytes << " B";
    }
    return ss.str();
}

void Window::resetStatistics() {
    // Reset session start time
    startTime = std::chrono::system_clock::now();

    // Update display to show reset state
    if (sessionTimeLabel) {
        gtk_label_set_text(sessionTimeLabel, "Session Time: 00:00:00");
    }
    if (avgSpeedLabel) {
        gtk_label_set_text(avgSpeedLabel, "Average: 0 B/s down, 0 B/s up");
    }
    if (totalLabel) {
        gtk_label_set_text(totalLabel, "Total: 0 B downloaded, 0 B uploaded");
    }
}

void Window::handleClose() {
    if (window) {
        gtk_widget_hide(window);
    }
}