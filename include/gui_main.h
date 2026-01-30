#ifndef GUI_MAIN_H
#define GUI_MAIN_H

#include <gtk/gtk.h>
#include "adb_abstraction.h"
#include "device_inspector.h"
#include "root_analyzer.h"
#include "bootloader_analyzer.h"
#include "rom_compatibility.h"
#include "config_manager.h"

// Application state
struct AppState {
    AdbAbstraction* adb;
    DeviceInspector* inspector;
    RootAnalyzer* root_analyzer;
    BootloaderAnalyzer* bootloader_analyzer;
    RomCompatibility* rom_compat;
    ConfigManager* config;
    
    GtkWidget* main_window;
    GtkWidget* device_list_combo;
    GtkWidget* device_info_text;
    GtkWidget* root_status_text;
    GtkWidget* bootloader_status_text;
    GtkWidget* rom_compat_text;
    GtkWidget* adb_path_entry;
    GtkWidget* status_bar;
    
    std::string selected_device;
};

// GUI initialization and callbacks
namespace GUI {
    // Initialize GTK application
    int init(int argc, char* argv[]);

    // Callbacks (C-style for GTK)
    extern "C" {
        void on_scan_devices(GtkButton* button, gpointer user_data);
        void on_device_selected(GtkComboBox* combo, gpointer user_data);
        void on_adb_path_changed(GtkEntry* entry, gpointer user_data);
        void on_refresh_info(GtkButton* button, gpointer user_data);
        void on_quit(GtkButton* button, gpointer user_data);
    }
}

#endif // GUI_MAIN_H
