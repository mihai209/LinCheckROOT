#include "gui_main.h"
#include <iostream>
#include <sstream>
#include <memory>

// Suppress deprecation warnings for GTK4 compatibility
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

// Global state
static std::unique_ptr<AppState> app_state = nullptr;

// Helper function to update status bar
static void update_status(const std::string& message)
{
    if (app_state && app_state->status_bar) {
        gtk_statusbar_remove_all(GTK_STATUSBAR(app_state->status_bar), 0);
        gtk_statusbar_push(GTK_STATUSBAR(app_state->status_bar), 0, message.c_str());
    }
}

// Callback: Scan devices button
extern "C" void on_scan_devices(GtkButton* button, gpointer user_data)
{
    if (!app_state) return;

    update_status("Scanning devices...");

    // Verify ADB is valid
    if (!app_state->adb->verify_adb()) {
        update_status("Error: ADB not found or not working");
        return;
    }

    // Get devices
    auto devices = app_state->adb->list_devices();

    // Clear combo box
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(app_state->device_list_combo));

    if (devices.empty()) {
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app_state->device_list_combo), nullptr, "No devices found");
        gtk_combo_box_set_active(GTK_COMBO_BOX(app_state->device_list_combo), 0);
        update_status("No devices found. Check USB connection and developer options.");
        return;
    }

    // Add devices to combo box
    for (size_t i = 0; i < devices.size(); ++i) {
        std::string label = devices[i].serial + " (" + devices[i].state_string + ")";
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(app_state->device_list_combo), 
                                  devices[i].serial.c_str(), label.c_str());
    }

    gtk_combo_box_set_active(GTK_COMBO_BOX(app_state->device_list_combo), 0);

    std::string msg = "Found " + std::to_string(devices.size()) + " device(s)";
    update_status(msg);
}

// Callback: Device selected
extern "C" void on_device_selected(GtkComboBox* combo, gpointer user_data)
{
    if (!app_state) return;

    const gchar* serial = gtk_combo_box_get_active_id(combo);
    if (!serial) return;

    app_state->selected_device = serial;

    // Clear text views
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_state->device_info_text)), "", -1);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_state->root_status_text)), "", -1);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_state->bootloader_status_text)), "", -1);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_state->rom_compat_text)), "", -1);

    update_status("Device selected. Click 'Refresh' to get information.");
}

// Callback: Refresh info button
extern "C" void on_refresh_info(GtkButton* button, gpointer user_data)
{
    if (!app_state || app_state->selected_device.empty()) {
        update_status("No device selected");
        return;
    }

    update_status("Analyzing device...");

    std::stringstream ss_info, ss_root, ss_bootloader, ss_rom;

    // Device info
    auto device_info = app_state->inspector->inspect(app_state->selected_device);
    if (device_info) {
        ss_info << "=== DEVICE INFORMATION ===\n\n";
        ss_info << "Manufacturer: " << device_info->manufacturer << "\n";
        ss_info << "Model: " << device_info->model << "\n";
        ss_info << "Codename: " << device_info->codename << "\n";
        ss_info << "Android Version: " << device_info->android_version << "\n";
        ss_info << "API Level: " << device_info->api_level << "\n";
        ss_info << "Build Fingerprint: " << device_info->build_fingerprint << "\n";
        ss_info << "\n=== HARDWARE ===\n\n";
        ss_info << "CPU ABI: " << device_info->cpu_abi << "\n";
        ss_info << "Architecture: " << device_info->arch << "\n";
        ss_info << "CPU Cores: " << device_info->cpu_cores << "\n";
        ss_info << "RAM: " << device_info->ram_mb << " MB\n";
        ss_info << "Storage Total: " << device_info->storage_total_mb << " MB\n";
        ss_info << "Storage Free: " << device_info->storage_free_mb << " MB\n";
        ss_info << "Kernel: " << device_info->kernel_version << "\n";
    } else {
        ss_info << "Error: Unable to get device information\n";
    }

    // Root status
    auto root_info = app_state->root_analyzer->analyze(app_state->selected_device);
    if (root_info) {
        ss_root << "=== ROOT STATUS ===\n\n";
        ss_root << "Status: " << app_state->root_analyzer->status_to_string(root_info->status) << "\n";
        ss_root << "Method: " << app_state->root_analyzer->method_to_string(root_info->method) << "\n";
        
        if (root_info->status != RootStatus::NOT_ROOTED) {
            if (!root_info->magisk_version.empty()) {
                ss_root << "Magisk Version: " << root_info->magisk_version << "\n";
            }
        }
    } else {
        ss_root << "Error: Unable to analyze root status\n";
    }

    // Bootloader status
    auto bootloader_info = app_state->bootloader_analyzer->analyze(app_state->selected_device);
    if (bootloader_info) {
        ss_bootloader << "=== BOOTLOADER STATUS ===\n\n";
        ss_bootloader << "Status: " << bootloader_info->status_string << "\n";
        ss_bootloader << "Fastboot Available: " << (bootloader_info->fastboot_available ? "Yes" : "No") << "\n\n";
        ss_bootloader << app_state->bootloader_analyzer->get_data_loss_warning() << "\n";
    } else {
        ss_bootloader << "Error: Unable to analyze bootloader status\n";
    }

    // ROM compatibility
    if (device_info) {
        auto rom_info = app_state->rom_compat->check_lineage_os(device_info->codename);
        if (rom_info) {
            ss_rom << "=== ROM COMPATIBILITY (LineageOS) ===\n\n";
            ss_rom << app_state->rom_compat->format_rom_info(rom_info.value());
        } else {
            ss_rom << "=== ROM COMPATIBILITY (LineageOS) ===\n\n";
            ss_rom << "Device '" << device_info->codename << "' not found in LineageOS database.\n";
            ss_rom << "This device may not be officially supported by LineageOS.\n";
        }
    } else {
        ss_rom << "Error: Unable to check ROM compatibility\n";
    }

    // Update text views
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_state->device_info_text)),
                            ss_info.str().c_str(), -1);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_state->root_status_text)),
                            ss_root.str().c_str(), -1);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_state->bootloader_status_text)),
                            ss_bootloader.str().c_str(), -1);
    gtk_text_buffer_set_text(gtk_text_view_get_buffer(GTK_TEXT_VIEW(app_state->rom_compat_text)),
                            ss_rom.str().c_str(), -1);

    update_status("Analysis complete");
}

// Callback: ADB path changed
extern "C" void on_adb_path_changed(GtkEntry* entry, gpointer user_data)
{
    if (!app_state) return;

    const gchar* path = gtk_editable_get_text(GTK_EDITABLE(entry));
    if (!path) return;

    std::string path_str(path);

    if (ConfigManager::verify_adb_path(path_str)) {
        // Reinitialize ADB with new path
        if (app_state->adb) {
            delete app_state->adb;
        }
        app_state->adb = new AdbAbstraction(path_str);

        // Save to config
        app_state->config->set("adb_path", path_str);
        app_state->config->save();

        update_status("ADB path updated and verified");
    } else {
        update_status("Error: Invalid ADB path");
    }
}

// Callback: Quit
extern "C" void on_quit(GtkButton* button, gpointer user_data)
{
    gtk_window_close(GTK_WINDOW(app_state->main_window));
}

// Build the GUI
static GtkWidget* build_gui(GtkApplication* app)
{
    // Main window
    GtkWidget* window = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 700);
    gtk_window_set_title(GTK_WINDOW(window), "LinCheckROOT - Android Device Analyzer");

    app_state->main_window = window;

    // Main container
    GtkWidget* main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(main_box, 10);
    gtk_widget_set_margin_bottom(main_box, 10);
    gtk_widget_set_margin_start(main_box, 10);
    gtk_widget_set_margin_end(main_box, 10);

    // Top control panel
    GtkWidget* control_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);

    // ADB path input
    GtkWidget* adb_label = gtk_label_new("ADB Path:");
    app_state->adb_path_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(app_state->adb_path_entry), 
                          app_state->config->get("adb_path").c_str());
    g_signal_connect(app_state->adb_path_entry, "changed", 
                     G_CALLBACK(on_adb_path_changed), nullptr);

    gtk_box_append(GTK_BOX(control_box), adb_label);
    gtk_box_append(GTK_BOX(control_box), app_state->adb_path_entry);

    // Scan button
    GtkWidget* scan_button = gtk_button_new_with_label("Scan Devices");
    g_signal_connect(scan_button, "clicked", G_CALLBACK(on_scan_devices), nullptr);
    gtk_box_append(GTK_BOX(control_box), scan_button);

    // Device selector
    GtkWidget* device_label = gtk_label_new("Device:");
    app_state->device_list_combo = gtk_combo_box_text_new();
    gtk_box_append(GTK_BOX(control_box), device_label);
    gtk_box_append(GTK_BOX(control_box), app_state->device_list_combo);
    g_signal_connect(app_state->device_list_combo, "changed", 
                     G_CALLBACK(on_device_selected), nullptr);

    gtk_box_append(GTK_BOX(main_box), control_box);

    // Refresh button
    GtkWidget* refresh_button = gtk_button_new_with_label("Refresh Information");
    g_signal_connect(refresh_button, "clicked", G_CALLBACK(on_refresh_info), nullptr);
    gtk_box_append(GTK_BOX(main_box), refresh_button);

    // Notebook for tabs
    GtkWidget* notebook = gtk_notebook_new();

    // Device Info Tab
    app_state->device_info_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app_state->device_info_text), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app_state->device_info_text), GTK_WRAP_WORD);

    GtkWidget* device_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(device_scroll), app_state->device_info_text);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), device_scroll, 
                            gtk_label_new("Device Info"));

    // Root Status Tab
    app_state->root_status_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app_state->root_status_text), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app_state->root_status_text), GTK_WRAP_WORD);

    GtkWidget* root_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(root_scroll), app_state->root_status_text);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), root_scroll, 
                            gtk_label_new("Root & Bootloader"));

    // Bootloader Status Tab
    app_state->bootloader_status_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app_state->bootloader_status_text), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app_state->bootloader_status_text), GTK_WRAP_WORD);

    GtkWidget* bootloader_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(bootloader_scroll), 
                                  app_state->bootloader_status_text);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), bootloader_scroll, 
                            gtk_label_new("Bootloader"));

    // ROM Compatibility Tab
    app_state->rom_compat_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app_state->rom_compat_text), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app_state->rom_compat_text), GTK_WRAP_WORD);

    GtkWidget* rom_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(rom_scroll), app_state->rom_compat_text);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), rom_scroll, 
                            gtk_label_new("ROM Compatibility"));

    gtk_box_append(GTK_BOX(main_box), notebook);

    // Status bar
    app_state->status_bar = gtk_statusbar_new();
    gtk_box_append(GTK_BOX(main_box), app_state->status_bar);

    // Quit button
    GtkWidget* quit_button = gtk_button_new_with_label("Quit");
    g_signal_connect(quit_button, "clicked", G_CALLBACK(on_quit), nullptr);
    gtk_box_append(GTK_BOX(main_box), quit_button);

    // Set main container
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    return window;
}

// GTK application activate callback
static void activate(GtkApplication* app, gpointer user_data)
{
    GtkWidget* window = build_gui(app);
    gtk_window_present(GTK_WINDOW(window));
}

int GUI::init(int argc, char* argv[])
{
    // Initialize application state
    app_state = std::make_unique<AppState>();

    // Initialize config
    app_state->config = new ConfigManager();
    app_state->config->load();

    // Get ADB path from config
    std::string adb_path = app_state->config->get("adb_path");

    // Initialize ADB
    app_state->adb = new AdbAbstraction(adb_path);

    if (!app_state->adb->verify_adb()) {
        std::cerr << "Warning: ADB not found. Please configure ADB path in GUI.\n";
    }

    // Initialize other modules
    app_state->inspector = new DeviceInspector(*app_state->adb);
    app_state->root_analyzer = new RootAnalyzer(*app_state->adb);
    app_state->bootloader_analyzer = new BootloaderAnalyzer(*app_state->adb);

    // Initialize ROM compatibility database
    app_state->rom_compat = new RomCompatibility();

    // Try to load database from standard locations
    const char* db_locations[] = {
        "/usr/share/lincheckroot/lineage_devices.json",
        "/usr/local/share/lincheckroot/lineage_devices.json",
        "./data/lineage_devices.json",
    };

    for (const auto& loc : db_locations) {
        if (app_state->rom_compat->load_database(loc)) {
            break;
        }
    }

    // Create GTK application
    GtkApplication* app = gtk_application_new("com.lincheckroot.app",
                                              G_APPLICATION_DEFAULT_FLAGS);

    g_signal_connect(app, "activate", G_CALLBACK(activate), nullptr);

    int status = g_application_run(G_APPLICATION(app), argc, argv);

    g_object_unref(app);

    // Cleanup
    delete app_state->adb;
    delete app_state->inspector;
    delete app_state->root_analyzer;
    delete app_state->bootloader_analyzer;
    delete app_state->rom_compat;
    delete app_state->config;

    return status;
}

#pragma GCC diagnostic pop
