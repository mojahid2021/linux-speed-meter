
#ifndef TRAY_ICON_H
#define TRAY_ICON_H

#include <string>
#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>

class TrayIcon {
public:
    TrayIcon();
    ~TrayIcon();
    void createTrayIcon();
    void set_label(const std::string& label, const std::string& tooltip);
    void set_menu(GtkWidget* menu);
    void cleanup();
private:
    AppIndicator* indicator;
};

#endif // TRAY_ICON_H