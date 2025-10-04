#include "../include/window.h"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <chrono>
#include <ctime>
#include <fstream>
#include <vector>

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
        gtk_window_set_default_size(GTK_WINDOW(window), 700, 600);

        // Create main vertical box
        GtkWidget* mainVBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_container_add(GTK_CONTAINER(window), mainVBox);
        gtk_container_set_border_width(GTK_CONTAINER(mainVBox), 10);

        // Title
        GtkWidget* titleLabel = gtk_label_new(NULL);
        gtk_label_set_markup(GTK_LABEL(titleLabel), "<span size='large' weight='bold'>Internet Speed Meter</span>");
        gtk_box_pack_start(GTK_BOX(mainVBox), titleLabel, FALSE, FALSE, 5);

        // Create notebook for tabbed interface
        GtkWidget* notebook = gtk_notebook_new();
        gtk_box_pack_start(GTK_BOX(mainVBox), notebook, TRUE, TRUE, 5);
        
        // Tab 1: Network Monitor
        GtkWidget* monitorTab = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
        gtk_container_set_border_width(GTK_CONTAINER(monitorTab), 10);
        
        // Current speeds section
        createSpeedSection(monitorTab);

        // Session statistics section
        createSessionStatsSection(monitorTab);

        // Monthly statistics section
        createMonthlyStatsSection(monitorTab);

        // Data limit section
        createDataLimitSection(monitorTab);

        // Network interface section
        createInterfaceSection(monitorTab);

        // Control buttons
        createButtonSection(monitorTab);
        
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook), monitorTab, 
                                gtk_label_new("Network Monitor"));
        
        // Tab 2: Speed Test
        speedTestWidget = std::make_unique<SpeedTestWidget>();
        GtkWidget* speedTestTab = speedTestWidget->create();
        gtk_notebook_append_page(GTK_NOTEBOOK(notebook), speedTestTab,
                                gtk_label_new("Speed Test"));

        // Connect the close event - prevent destruction, just hide
        g_signal_connect(window, "delete-event", G_CALLBACK(+[](GtkWidget*, GdkEvent*, gpointer self) {
            static_cast<Window*>(self)->handleClose();
            return TRUE; // Prevent window destruction
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

void Window::createMonthlyStatsSection(GtkWidget* parent) {
    GtkWidget* frame = gtk_frame_new("Monthly Statistics");
    gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 5);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(frame), vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    // Monthly download
    monthlyDownloadLabel = GTK_LABEL(gtk_label_new("Monthly Download: 0 MB"));
    gtk_label_set_xalign(GTK_LABEL(monthlyDownloadLabel), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(monthlyDownloadLabel), FALSE, FALSE, 2);

    // Monthly upload
    monthlyUploadLabel = GTK_LABEL(gtk_label_new("Monthly Upload: 0 MB"));
    gtk_label_set_xalign(GTK_LABEL(monthlyUploadLabel), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(monthlyUploadLabel), FALSE, FALSE, 2);

    // Peak speeds this month
    monthlyPeakDownloadLabel = GTK_LABEL(gtk_label_new("Peak Download: 0 MB/s"));
    gtk_label_set_xalign(GTK_LABEL(monthlyPeakDownloadLabel), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(monthlyPeakDownloadLabel), FALSE, FALSE, 2);

    monthlyPeakUploadLabel = GTK_LABEL(gtk_label_new("Peak Upload: 0 MB/s"));
    gtk_label_set_xalign(GTK_LABEL(monthlyPeakUploadLabel), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(monthlyPeakUploadLabel), FALSE, FALSE, 2);
}

void Window::createDataLimitSection(GtkWidget* parent) {
    GtkWidget* frame = gtk_frame_new("Data Usage & Limits");
    gtk_box_pack_start(GTK_BOX(parent), frame, FALSE, FALSE, 5);

    GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(frame), vbox);
    gtk_container_set_border_width(GTK_CONTAINER(vbox), 10);

    // Data usage
    dataUsageLabel = GTK_LABEL(gtk_label_new("Data Used: 0% of limit"));
    gtk_label_set_xalign(GTK_LABEL(dataUsageLabel), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(dataUsageLabel), FALSE, FALSE, 2);

    // Data limit status
    dataLimitStatusLabel = GTK_LABEL(gtk_label_new("Status: No limit set"));
    gtk_label_set_xalign(GTK_LABEL(dataLimitStatusLabel), 0.0);
    gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(dataLimitStatusLabel), FALSE, FALSE, 2);
}

void Window::createButtonSection(GtkWidget* parent) {
    GtkWidget* hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(parent), hbox, FALSE, FALSE, 5);

    // Export CSV button
    GtkWidget* exportCSVButton = gtk_button_new_with_label("Export CSV");
    g_signal_connect(exportCSVButton, "clicked", G_CALLBACK(+[](GtkWidget*, gpointer self) {
        static_cast<Window*>(self)->exportToCSV();
    }), this);
    gtk_box_pack_start(GTK_BOX(hbox), exportCSVButton, FALSE, FALSE, 0);

    // Export JSON button
    GtkWidget* exportJSONButton = gtk_button_new_with_label("Export JSON");
    g_signal_connect(exportJSONButton, "clicked", G_CALLBACK(+[](GtkWidget*, gpointer self) {
        static_cast<Window*>(self)->exportToJSON();
    }), this);
    gtk_box_pack_start(GTK_BOX(hbox), exportJSONButton, FALSE, FALSE, 0);

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
        totalText << formatBytes(totalDownload) << " / " << formatBytes(totalUpload);
        gtk_label_set_text(totalLabel, totalText.str().c_str());
    }
    
    // Record usage data for export
    UsageData data;
    data.timestamp = std::chrono::system_clock::now();
    data.downloadSpeed = downloadSpeed;
    data.uploadSpeed = uploadSpeed;
    data.totalDownload = totalDownload;
    data.totalUpload = totalUpload;
    
    usageHistory.push_back(data);
    
    // Keep only last 1000 records
    if (usageHistory.size() > 1000) {
        usageHistory.erase(usageHistory.begin());
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
    updateMonthlyStats();
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

void Window::updateMonthlyStats() {
    if (!dataManager) return;

    MonthlyStats monthly = dataManager->getCurrentMonthStats();

    // Update monthly download
    if (monthlyDownloadLabel) {
        std::stringstream text;
        text << "Monthly Download: " << formatBytes(monthly.total_download_bytes);
        gtk_label_set_text(monthlyDownloadLabel, text.str().c_str());
    }

    // Update monthly upload
    if (monthlyUploadLabel) {
        std::stringstream text;
        text << "Monthly Upload: " << formatBytes(monthly.total_upload_bytes);
        gtk_label_set_text(monthlyUploadLabel, text.str().c_str());
    }

    // Update peak download
    if (monthlyPeakDownloadLabel) {
        std::stringstream text;
        text << "Peak Download: " << formatSpeedSimple(monthly.peak_download_speed);
        gtk_label_set_text(monthlyPeakDownloadLabel, text.str().c_str());
    }

    // Update peak upload
    if (monthlyPeakUploadLabel) {
        std::stringstream text;
        text << "Peak Upload: " << formatSpeedSimple(monthly.peak_upload_speed);
        gtk_label_set_text(monthlyPeakUploadLabel, text.str().c_str());
    }

    // Update data usage
    if (dataUsageLabel) {
        double usagePercent = dataManager->getDataUsagePercentage();
        std::stringstream text;
        text << "Data Used: " << std::fixed << std::setprecision(1) << usagePercent << "% of limit";
        gtk_label_set_text(dataUsageLabel, text.str().c_str());
    }

    // Update data limit status
    if (dataLimitStatusLabel) {
        uint64_t limit = dataManager->getDataLimit();
        if (limit > 0) {
            bool exceeded = dataManager->isDataLimitExceeded();
            std::stringstream text;
            text << "Status: " << (exceeded ? "LIMIT EXCEEDED!" : "Within limit");
            gtk_label_set_text(dataLimitStatusLabel, text.str().c_str());

            // Change color if exceeded
            if (exceeded) {
                gtk_label_set_markup(GTK_LABEL(dataLimitStatusLabel),
                    "<span foreground='red' weight='bold'>Status: LIMIT EXCEEDED!</span>");
            }
        } else {
            gtk_label_set_text(dataLimitStatusLabel, "Status: No limit set");
        }
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

void Window::exportToCSV() {
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Export to CSV",
        GTK_WINDOW(window),
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Save", GTK_RESPONSE_ACCEPT,
        NULL);
    
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "speed_meter_data.csv");
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        std::ofstream file(filename);
        if (file.is_open()) {
            // Write CSV header
            file << "Timestamp,Download Speed (MB/s),Upload Speed (MB/s),Total Download (MB),Total Upload (MB)\n";
            
            // Write data
            for (const auto& data : usageHistory) {
                auto time_t = std::chrono::system_clock::to_time_t(data.timestamp);
                struct tm* tm_info = std::localtime(&time_t);
                char timeStr[64];
                std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
                
                file << timeStr << ","
                     << std::fixed << std::setprecision(3) << (data.downloadSpeed / (1024.0 * 1024.0)) << ","
                     << std::fixed << std::setprecision(3) << (data.uploadSpeed / (1024.0 * 1024.0)) << ","
                     << std::fixed << std::setprecision(2) << (data.totalDownload / (1024.0 * 1024.0)) << ","
                     << std::fixed << std::setprecision(2) << (data.totalUpload / (1024.0 * 1024.0)) << "\n";
            }
            
            file.close();
            
            GtkWidget* msgDialog = gtk_message_dialog_new(GTK_WINDOW(window),
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_INFO,
                GTK_BUTTONS_OK,
                "Data exported successfully to CSV");
            gtk_dialog_run(GTK_DIALOG(msgDialog));
            gtk_widget_destroy(msgDialog);
        } else {
            GtkWidget* errorDialog = gtk_message_dialog_new(GTK_WINDOW(window),
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "Failed to export data to CSV");
            gtk_dialog_run(GTK_DIALOG(errorDialog));
            gtk_widget_destroy(errorDialog);
        }
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}

void Window::exportToJSON() {
    GtkWidget* dialog = gtk_file_chooser_dialog_new("Export to JSON",
        GTK_WINDOW(window),
        GTK_FILE_CHOOSER_ACTION_SAVE,
        "_Cancel", GTK_RESPONSE_CANCEL,
        "_Save", GTK_RESPONSE_ACCEPT,
        NULL);
    
    gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), "speed_meter_data.json");
    
    if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
        char* filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        
        std::ofstream file(filename);
        if (file.is_open()) {
            // Write JSON
            auto now = std::chrono::system_clock::now();
            auto time_t = std::chrono::system_clock::to_time_t(now);
            struct tm* tm_info = std::localtime(&time_t);
            char exportTime[64];
            std::strftime(exportTime, sizeof(exportTime), "%Y-%m-%d %H:%M:%S", tm_info);
            
            file << "{\n";
            file << "  \"exportDate\": \"" << exportTime << "\",\n";
            file << "  \"recordCount\": " << usageHistory.size() << ",\n";
            file << "  \"data\": [\n";
            
            for (size_t i = 0; i < usageHistory.size(); ++i) {
                const auto& data = usageHistory[i];
                auto data_time_t = std::chrono::system_clock::to_time_t(data.timestamp);
                struct tm* data_tm = std::localtime(&data_time_t);
                char timeStr[64];
                std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", data_tm);
                
                file << "    {\n";
                file << "      \"timestamp\": \"" << timeStr << "\",\n";
                file << "      \"downloadSpeedMBps\": " << std::fixed << std::setprecision(3) 
                     << (data.downloadSpeed / (1024.0 * 1024.0)) << ",\n";
                file << "      \"uploadSpeedMBps\": " << std::fixed << std::setprecision(3) 
                     << (data.uploadSpeed / (1024.0 * 1024.0)) << ",\n";
                file << "      \"totalDownloadMB\": " << std::fixed << std::setprecision(2) 
                     << (data.totalDownload / (1024.0 * 1024.0)) << ",\n";
                file << "      \"totalUploadMB\": " << std::fixed << std::setprecision(2) 
                     << (data.totalUpload / (1024.0 * 1024.0)) << "\n";
                file << "    }" << (i < usageHistory.size() - 1 ? "," : "") << "\n";
            }
            
            file << "  ]\n";
            file << "}\n";
            
            file.close();
            
            GtkWidget* msgDialog = gtk_message_dialog_new(GTK_WINDOW(window),
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_INFO,
                GTK_BUTTONS_OK,
                "Data exported successfully to JSON");
            gtk_dialog_run(GTK_DIALOG(msgDialog));
            gtk_widget_destroy(msgDialog);
        } else {
            GtkWidget* errorDialog = gtk_message_dialog_new(GTK_WINDOW(window),
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "Failed to export data to JSON");
            gtk_dialog_run(GTK_DIALOG(errorDialog));
            gtk_widget_destroy(errorDialog);
        }
        
        g_free(filename);
    }
    
    gtk_widget_destroy(dialog);
}
