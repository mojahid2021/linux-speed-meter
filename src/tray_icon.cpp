#include "../include/tray_icon.h"
#include <iostream>

TrayIcon::TrayIcon() : indicator(nullptr) {}

TrayIcon::~TrayIcon() {
    cleanup();
}

void TrayIcon::createTrayIcon() {
    indicator = app_indicator_new("speed-meter", "network-transmit-receive", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
    app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
    app_indicator_set_label(indicator, "Speed Meter", "Initializing...");
}

void TrayIcon::set_label(const std::string& label, const std::string& tooltip) {
    if (indicator) {
        app_indicator_set_label(indicator, label.c_str(), tooltip.c_str());
    }
}

void TrayIcon::set_menu(GtkWidget* menu) {
    if (indicator) {
        app_indicator_set_menu(indicator, GTK_MENU(menu));
    }
}

void TrayIcon::cleanup() {
    if (indicator) {
        g_object_unref(indicator);
        indicator = nullptr;
    }
}