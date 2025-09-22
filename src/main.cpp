#include <gtk/gtk.h>
// #include <gtkmm.h>
#include <csignal>
#include <memory>
#include <iostream>
#include "../include/tray_icon.h"
#include "../include/speed_monitor.h"
#include <fstream>
#include <sys/stat.h>
#include <cstdlib>
#include <string>

static std::atomic<bool> global_running(true);
void signal_handler(int) {
    global_running = false;
    gtk_main_quit();
}

TrayIcon trayIcon;
std::unique_ptr<SpeedMeter> speedMeter;

gboolean update_tray(gpointer) {
    if (speedMeter && global_running) {
        // The SpeedMeter thread updates stats and label/tooltip
        trayIcon.set_label(speedMeter->get_label(), speedMeter->get_tooltip());
    }
    return TRUE;
}

void on_quit(GtkMenuItem*, gpointer) {
    if (speedMeter) speedMeter->on_quit();
    global_running = false;
    gtk_main_quit();
}

void ensure_autostart(bool enable) {
    std::string autostart_path = std::string(getenv("HOME")) + "/.config/autostart/speed-meter.desktop";
    if (enable) {
        struct stat buffer;
        if (stat(autostart_path.c_str(), &buffer) != 0) {
            std::ofstream out(autostart_path);
            out << "[Desktop Entry]\n"
                   "Type=Application\n"
                   "Name=Internet Speed Meter\n"
                   "Exec=speed-meter\n"
                   "Icon=network-transmit-receive\n"
                   "Categories=Utility;Network;\n"
                   "StartupNotify=false\n";
            out.close();
        }
    } else {
        remove(autostart_path.c_str());
    }
}

int main(int argc, char *argv[]) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    gtk_init(&argc, &argv);
    trayIcon.createTrayIcon();

    try {
        speedMeter = std::make_unique<SpeedMeter>();
    } catch (const std::exception& e) {
        std::cerr << "Failed to initialize SpeedMeter: " << e.what() << std::endl;
        return 1;
    }

    ensure_autostart(true);

    // Create tray menu with Quit
    GtkWidget* menu = gtk_menu_new();
    GtkWidget* quit_item = gtk_menu_item_new_with_label("Quit");
    g_signal_connect(G_OBJECT(quit_item), "activate", G_CALLBACK(on_quit), NULL);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), quit_item);
    gtk_widget_show_all(menu);
    trayIcon.set_menu(menu);

    g_timeout_add_seconds(1, update_tray, NULL);

    gtk_main();

    return 0;
}