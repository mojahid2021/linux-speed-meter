#include <gtk/gtk.h>
// #include <gtkmm.h>
#include <csignal>
#include <memory>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <cstdlib>
#include <string>
#include <unistd.h>
#include <cstring>
#include <curl/curl.h>
#include "../include/tray_icon.h"
#include "../include/speed_monitor.h"
#include "../include/window.h"
#include "../include/data_manager.h"

// Forward declarations for auto-startup functions
void setup_autostart_linux();
void remove_autostart_linux();
void setup_xdg_autostart();
void setup_systemd_service();
void setup_cron_autostart();
void setup_desktop_specific_autostart();
void remove_xdg_autostart();
void remove_systemd_service();
void remove_cron_autostart();
void remove_desktop_specific_autostart();
void setup_kde_autostart();
void setup_gnome_autostart();
void setup_xfce_autostart();
void setup_lxde_autostart();
void setup_mate_autostart();
void setup_cinnamon_autostart();
void remove_kde_autostart();
void remove_gnome_autostart();
void remove_xfce_autostart();
void remove_lxde_autostart();
void remove_mate_autostart();
void remove_cinnamon_autostart();

static std::atomic<bool> global_running(true);
void signal_handler(int) {
    global_running = false;
    gtk_main_quit();
}

TrayIcon trayIcon;
std::unique_ptr<SpeedMeter> speedMeter;
std::unique_ptr<Window> dashboardWindow;
std::unique_ptr<DataManager> dataManager;

static int update_counter = 0;
static uint64_t prev_total_download_bytes = 0;
static uint64_t prev_total_upload_bytes = 0;

gboolean update_tray(gpointer) {
    if (speedMeter && global_running) {
        // The SpeedMeter thread updates stats and label/tooltip
        trayIcon.set_label(speedMeter->get_label(), speedMeter->get_tooltip());

        // Update dashboard window if it exists and is visible
        if (dashboardWindow) {
            dashboardWindow->updateSpeeds(
                speedMeter->get_current_upload_speed(),     // Current upload speed in bytes/sec
                speedMeter->get_current_download_speed(),   // Current download speed in bytes/sec
                speedMeter->get_total_tx_mb() * 1024 * 1024, // Total upload in bytes
                speedMeter->get_total_rx_mb() * 1024 * 1024, // Total download in bytes
                speedMeter->get_iface(),                     // Interface name
                "",                                          // IP address (not available in GTK version)
                true                                         // Assume connected if we have stats
            );
        }

        // Save data every 60 seconds (1 minute)
        update_counter++;
        if (update_counter >= 60 && dataManager) {
            uint64_t current_download_bytes = speedMeter->get_total_rx_mb() * 1024 * 1024;
            uint64_t current_upload_bytes = speedMeter->get_total_tx_mb() * 1024 * 1024;
            
            // Calculate incremental bytes since last save
            uint64_t incremental_download = current_download_bytes - prev_total_download_bytes;
            uint64_t incremental_upload = current_upload_bytes - prev_total_upload_bytes;
            
            // Update previous totals
            prev_total_download_bytes = current_download_bytes;
            prev_total_upload_bytes = current_upload_bytes;
            
            dataManager->updateDailyStats(
                incremental_download,                          // Incremental download bytes
                incremental_upload,                            // Incremental upload bytes
                speedMeter->get_current_download_speed(),
                speedMeter->get_current_upload_speed(),
                std::chrono::seconds(update_counter)          // Session time
            );
            update_counter = 0; // Reset counter
        }
    }
    return TRUE;
}

void on_show_dashboard(GtkMenuItem*, gpointer) {
    if (!dashboardWindow) {
        dashboardWindow = std::make_unique<Window>();
        dashboardWindow->setDataManager(dataManager.get());
    }
    dashboardWindow->show();
}

void on_quit(GtkMenuItem*, gpointer) {
    if (speedMeter) speedMeter->on_quit();
    global_running = false;
    gtk_main_quit();
}

void ensure_autostart(bool enable) {
    if (enable) {
        setup_autostart_linux();
    } else {
        remove_autostart_linux();
    }
}

void setup_autostart_linux() {
    // Method 1: XDG Autostart (works for most desktop environments)
    setup_xdg_autostart();

    // Method 2: System-wide systemd service (for servers/headless systems)
    setup_systemd_service();

    // Method 3: Cron job as fallback
    setup_cron_autostart();

    // Method 4: Desktop-specific autostart
    setup_desktop_specific_autostart();
}

void remove_autostart_linux() {
    remove_xdg_autostart();
    remove_systemd_service();
    remove_cron_autostart();
    remove_desktop_specific_autostart();
}

void setup_xdg_autostart() {
    std::string autostart_dir = std::string(getenv("HOME")) + "/.config/autostart";
    std::string desktop_file = autostart_dir + "/speed-meter.desktop";

    // Create autostart directory if it doesn't exist
    mkdir(autostart_dir.c_str(), 0755);

    struct stat buffer;
    if (stat(desktop_file.c_str(), &buffer) != 0) {
        std::ofstream out(desktop_file);
        if (out.is_open()) {
            out << "[Desktop Entry]\n"
                   "Type=Application\n"
                   "Name=Internet Speed Meter\n"
                   "Comment=Network speed monitoring application\n"
                   "Exec=speed-meter\n"
                   "Icon=network-transmit-receive\n"
                   "Categories=Utility;Network;Monitor;\n"
                   "StartupNotify=false\n"
                   "Terminal=false\n"
                   "X-GNOME-Autostart-enabled=true\n"
                   "X-KDE-autostart-after=panel\n"
                   "X-MATE-Autostart-Delay=0\n";
            out.close();
            std::cout << "Created XDG autostart entry: " << desktop_file << std::endl;
        }
    }
}

void remove_xdg_autostart() {
    std::string desktop_file = std::string(getenv("HOME")) + "/.config/autostart/speed-meter.desktop";
    if (remove(desktop_file.c_str()) == 0) {
        std::cout << "Removed XDG autostart entry" << std::endl;
    }
}

void setup_systemd_service() {
    // Check if systemd is available
    if (system("which systemctl > /dev/null 2>&1") != 0) {
        return; // systemd not available
    }

    std::string service_file = "/etc/systemd/system/speed-meter.service";
    std::string user_service_file = std::string(getenv("HOME")) + "/.config/systemd/user/speed-meter.service";

    // Try user service first (recommended for desktop apps)
    struct stat buffer;
    if (stat(user_service_file.c_str(), &buffer) != 0) {
        // Create user systemd directory
        std::string systemd_dir = std::string(getenv("HOME")) + "/.config/systemd/user";
        mkdir(systemd_dir.c_str(), 0755);

        std::ofstream out(user_service_file);
        if (out.is_open()) {
            out << "[Unit]\n"
                   "Description=Internet Speed Meter\n"
                   "After=network.target\n"
                   "\n"
                   "[Service]\n"
                   "Type=simple\n"
                   "ExecStart=/usr/local/bin/speed-meter\n"
                   "Restart=always\n"
                   "RestartSec=5\n"
                   "Environment=DISPLAY=:0\n"
                   "\n"
                   "[Install]\n"
                   "WantedBy=default.target\n";
            out.close();

            // Enable and start the service
            std::string cmd = "systemctl --user enable speed-meter.service";
            (void)system(cmd.c_str());
            cmd = "systemctl --user start speed-meter.service";
            (void)system(cmd.c_str());

            std::cout << "Created and enabled systemd user service" << std::endl;
        }
    }
}

void remove_systemd_service() {
    std::string user_service_file = std::string(getenv("HOME")) + "/.config/systemd/user/speed-meter.service";

    if (access(user_service_file.c_str(), F_OK) != -1) {
        // Stop and disable the service
        std::string cmd = "systemctl --user stop speed-meter.service 2>/dev/null";
        (void)system(cmd.c_str());
        cmd = "systemctl --user disable speed-meter.service 2>/dev/null";
        (void)system(cmd.c_str());

        // Remove the service file
        remove(user_service_file.c_str());
        std::cout << "Removed systemd user service" << std::endl;
    }
}

void setup_cron_autostart() {
    // Check if cron is available
    if (system("which crontab > /dev/null 2>&1") != 0) {
        return; // cron not available
    }

    // Add @reboot entry to crontab
    std::string cron_cmd = "(crontab -l 2>/dev/null; echo \"@reboot sleep 30 && /usr/local/bin/speed-meter\") | crontab -";
    if (system(cron_cmd.c_str()) == 0) {
        std::cout << "Added cron @reboot entry" << std::endl;
    }
}

void remove_cron_autostart() {
    // Remove the @reboot entry from crontab
    std::string cron_cmd = "crontab -l 2>/dev/null | grep -v 'speed-meter' | crontab -";
    (void)system(cron_cmd.c_str());
    std::cout << "Removed cron @reboot entry" << std::endl;
}

void setup_desktop_specific_autostart() {
    // Detect desktop environment
    const char* desktop = getenv("XDG_CURRENT_DESKTOP");
    const char* session = getenv("DESKTOP_SESSION");

    if (!desktop && !session) {
        return; // Cannot detect desktop environment
    }

    std::string desktop_env = desktop ? desktop : session;

    // KDE Plasma
    if (desktop_env.find("KDE") != std::string::npos ||
        desktop_env.find("plasma") != std::string::npos) {
        setup_kde_autostart();
    }
    // GNOME
    else if (desktop_env.find("GNOME") != std::string::npos) {
        setup_gnome_autostart();
    }
    // XFCE
    else if (desktop_env.find("XFCE") != std::string::npos) {
        setup_xfce_autostart();
    }
    // LXDE/LXQt
    else if (desktop_env.find("LXDE") != std::string::npos ||
             desktop_env.find("LXQt") != std::string::npos) {
        setup_lxde_autostart();
    }
    // MATE
    else if (desktop_env.find("MATE") != std::string::npos) {
        setup_mate_autostart();
    }
    // Cinnamon
    else if (desktop_env.find("Cinnamon") != std::string::npos) {
        setup_cinnamon_autostart();
    }
}

void remove_desktop_specific_autostart() {
    remove_kde_autostart();
    remove_gnome_autostart();
    remove_xfce_autostart();
    remove_lxde_autostart();
    remove_mate_autostart();
    remove_cinnamon_autostart();
}

void setup_kde_autostart() {
    std::string kde_autostart_dir = std::string(getenv("HOME")) + "/.kde/Autostart";
    std::string desktop_file = kde_autostart_dir + "/speed-meter.desktop";

    mkdir(kde_autostart_dir.c_str(), 0755);

    struct stat buffer;
    if (stat(desktop_file.c_str(), &buffer) != 0) {
        std::ofstream out(desktop_file);
        if (out.is_open()) {
            out << "[Desktop Entry]\n"
                   "Type=Application\n"
                   "Name=Internet Speed Meter\n"
                   "Exec=speed-meter\n"
                   "Icon=network-transmit-receive\n"
                   "X-KDE-autostart-after=panel\n";
            out.close();
            std::cout << "Created KDE autostart entry" << std::endl;
        }
    }
}

void remove_kde_autostart() {
    std::string desktop_file = std::string(getenv("HOME")) + "/.kde/Autostart/speed-meter.desktop";
    remove(desktop_file.c_str());
}

void setup_gnome_autostart() {
    // GNOME uses XDG autostart, which is already handled
    std::cout << "GNOME uses XDG autostart (already configured)" << std::endl;
}

void remove_gnome_autostart() {
    // GNOME cleanup is handled by XDG removal
}

void setup_xfce_autostart() {
    std::string xfce_autostart_dir = std::string(getenv("HOME")) + "/.config/xfce4/autostart";
    std::string desktop_file = xfce_autostart_dir + "/speed-meter.desktop";

    mkdir(xfce_autostart_dir.c_str(), 0755);

    struct stat buffer;
    if (stat(desktop_file.c_str(), &buffer) != 0) {
        std::ofstream out(desktop_file);
        if (out.is_open()) {
            out << "[Desktop Entry]\n"
                   "Type=Application\n"
                   "Name=Internet Speed Meter\n"
                   "Exec=speed-meter\n"
                   "Icon=network-transmit-receive\n";
            out.close();
            std::cout << "Created XFCE autostart entry" << std::endl;
        }
    }
}

void remove_xfce_autostart() {
    std::string desktop_file = std::string(getenv("HOME")) + "/.config/xfce4/autostart/speed-meter.desktop";
    remove(desktop_file.c_str());
}

void setup_lxde_autostart() {
    std::string lxde_autostart_file = std::string(getenv("HOME")) + "/.config/lxsession/LXDE/autostart";

    std::ofstream out(lxde_autostart_file, std::ios::app);
    if (out.is_open()) {
        // Check if already exists
        std::ifstream in(lxde_autostart_file);
        std::string line;
        bool exists = false;
        while (std::getline(in, line)) {
            if (line.find("speed-meter") != std::string::npos) {
                exists = true;
                break;
            }
        }

        if (!exists) {
            out << "@speed-meter\n";
            std::cout << "Added LXDE autostart entry" << std::endl;
        }
    }
}

void remove_lxde_autostart() {
    std::string lxde_autostart_file = std::string(getenv("HOME")) + "/.config/lxsession/LXDE/autostart";

    std::ifstream in(lxde_autostart_file);
    std::string content;
    std::string line;
    while (std::getline(in, line)) {
        if (line.find("speed-meter") == std::string::npos) {
            content += line + "\n";
        }
    }

    std::ofstream out(lxde_autostart_file);
    out << content;
}

void setup_mate_autostart() {
    std::string mate_autostart_dir = std::string(getenv("HOME")) + "/.config/autostart";
    // MATE uses XDG autostart, which is already handled
    std::cout << "MATE uses XDG autostart (already configured)" << std::endl;
}

void remove_mate_autostart() {
    // MATE cleanup is handled by XDG removal
}

void setup_cinnamon_autostart() {
    std::string cinnamon_autostart_dir = std::string(getenv("HOME")) + "/.config/autostart";
    // Cinnamon uses XDG autostart, which is already handled
    std::cout << "Cinnamon uses XDG autostart (already configured)" << std::endl;
}

void remove_cinnamon_autostart() {
    // Cinnamon cleanup is handled by XDG removal
}

int main(int argc, char *argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    // Initialize CURL globally (thread-safe initialization)
    curl_global_init(CURL_GLOBAL_ALL);

    gtk_init(&argc, &argv);
    trayIcon.createTrayIcon();

    try {
        speedMeter = std::make_unique<SpeedMeter>();
        dataManager = std::make_unique<DataManager>();
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize SpeedMeter or DataManager: " << e.what() << std::endl;
        return 1;
    }

    ensure_autostart(true);

    // Create tray menu
    GtkWidget* menu = gtk_menu_new();

    // Show Dashboard menu item
    GtkWidget* dashboard_item = gtk_menu_item_new_with_label("Show Dashboard");
    g_signal_connect(G_OBJECT(dashboard_item), "activate", G_CALLBACK(on_show_dashboard), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), dashboard_item);

    // Separator
    GtkWidget* separator = gtk_separator_menu_item_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);

    // Quit menu item
    GtkWidget* quit_item = gtk_menu_item_new_with_label("Quit");
    g_signal_connect(G_OBJECT(quit_item), "activate", G_CALLBACK(on_quit), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), quit_item);

    gtk_widget_show_all(menu);
    trayIcon.set_menu(menu);

    g_timeout_add_seconds(1, update_tray, NULL);

    gtk_main();

    // Cleanup CURL globally
    curl_global_cleanup();

    return 0;
}