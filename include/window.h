#ifndef WINDOW_H
#define WINDOW_H
#include <gtk/gtk.h>
class Window {
public:
    Window();
    ~Window();
    void show();
    void updateSpeeds(double uploadSpeed, double downloadSpeed);
    void handleClose();
private:
    GtkWidget* window;
    GtkLabel* uploadLabel;
    GtkLabel* downloadLabel;
};
#endif // WINDOW_H
