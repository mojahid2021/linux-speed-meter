#include "../include/window.h"
#include <iostream>

Window::Window() : window(nullptr), uploadLabel(nullptr), downloadLabel(nullptr) {}

Window::~Window() {
    if (window) {
        gtk_widget_destroy(window);
    }
}

void Window::show() {
    if (!window) {
        // Initialize the GTK window
        window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
        gtk_window_set_title(GTK_WINDOW(window), "Internet Speed Meter");
        gtk_window_set_default_size(GTK_WINDOW(window), 300, 100);

        // Create a vertical box to hold the labels
        GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
        gtk_container_add(GTK_CONTAINER(window), vbox);

        // Create labels for upload and download speeds
        uploadLabel = GTK_LABEL(gtk_label_new("Upload Speed: 0 KB/s"));
        downloadLabel = GTK_LABEL(gtk_label_new("Download Speed: 0 KB/s"));

        // Add labels to the box
        gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(uploadLabel), TRUE, TRUE, 0);
        gtk_box_pack_start(GTK_BOX(vbox), GTK_WIDGET(downloadLabel), TRUE, TRUE, 0);

        // Connect the close event
        g_signal_connect(window, "destroy", G_CALLBACK(+[](GtkWidget*, gpointer self) {
            static_cast<Window*>(self)->handleClose();
        }), this);

        gtk_widget_show_all(window);
    } else {
        gtk_widget_show(window);
    }
}

void Window::updateSpeeds(double uploadSpeed, double downloadSpeed) {
    if (uploadLabel && downloadLabel) {
        // Update the labels with the current speeds
        gchar uploadText[50];
        gchar downloadText[50];
        g_snprintf(uploadText, sizeof(uploadText), "Upload Speed: %.2f KB/s", uploadSpeed);
        g_snprintf(downloadText, sizeof(downloadText), "Download Speed: %.2f KB/s", downloadSpeed);

        gtk_label_set_text(uploadLabel, uploadText);
        gtk_label_set_text(downloadLabel, downloadText);
    }
}

void Window::handleClose() {
    if (window) {
        gtk_widget_hide(window);
    }
}