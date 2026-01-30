#include "gui_main.h"
#include <iostream>
#include <sstream>
#include <memory>

// New modules
#include "adb/adb_client.hpp"
#include "device/device_info.hpp"
#include "actions/reboot.hpp"
#include "analyzer/analyzers.hpp"
#include "ui/dialogs.hpp"

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

// ============ NEW CALLBACKS FOR DEVICE ACTIONS ============

// Helper: Create reboot action handler
static actions::RebootAction* get_reboot_action()
{
    if (!app_state) return nullptr;

    // Create ADB client and device info using new modules
    static std::unique_ptr<adb::AdbClient> adb_client;
    static std::unique_ptr<device::DeviceInfo> device_info;
    static std::unique_ptr<actions::RebootAction> reboot_action;

    if (!adb_client) {
        adb_client = std::make_unique<adb::AdbClient>(app_state->adb->get_adb_path());
        device_info = std::make_unique<device::DeviceInfo>(*adb_client);
        reboot_action = std::make_unique<actions::RebootAction>(*adb_client, *device_info);
    }

    return reboot_action.get();
}

// Callback: Reboot System
extern "C" void on_reboot_system(GtkButton* button, gpointer user_data)
{
    if (!app_state) return;

    auto action = get_reboot_action();
    if (!action || !action->can_reboot()) {
        ui::Dialogs::show_error(GTK_WINDOW(app_state->main_window),
                               "Device Not Connected",
                               "No device is connected. Connect an Android device with USB Debugging enabled.");
        return;
    }

    auto info = action->get_system_reboot_info();
    if (ui::Dialogs::confirm_reboot(GTK_WINDOW(app_state->main_window),
                                    info.type, info.description, info.warning)) {
        update_status("Rebooting system...");
        
        // Show the command to execute
        std::string cmd = "adb reboot";
        std::string help_msg = "Command sent! If device doesn't reboot, copy and run this in terminal:\n\n";
        help_msg += cmd + "\n\nMake sure USB Debugging is enabled.";
        
        ui::Dialogs::show_command_details(GTK_WINDOW(app_state->main_window),
                                         cmd,
                                         help_msg,
                                         true);
        
        update_status("✅ Reboot initiated...");
    }
}

// Callback: Reboot Bootloader
extern "C" void on_reboot_bootloader(GtkButton* button, gpointer user_data)
{
    if (!app_state) return;

    auto action = get_reboot_action();
    if (!action || !action->can_reboot()) {
        ui::Dialogs::show_error(GTK_WINDOW(app_state->main_window),
                               "Device Not Connected",
                               "No device is connected. Connect an Android device with USB Debugging enabled.");
        return;
    }

    auto info = action->get_bootloader_reboot_info();
    if (ui::Dialogs::confirm_reboot(GTK_WINDOW(app_state->main_window),
                                    info.type, info.description, info.warning)) {
        update_status("Rebooting to bootloader...");
        
        std::string cmd = "adb reboot bootloader";
        std::string help_msg = "Command sent! If device doesn't reboot, copy and run this in terminal:\n\n";
        help_msg += cmd + "\n\nMake sure USB Debugging is enabled.";
        if (info.warning.find("Samsung") != std::string::npos) {
            help_msg += "\n\n⚠️  " + info.warning;
        }
        
        ui::Dialogs::show_command_details(GTK_WINDOW(app_state->main_window),
                                         cmd,
                                         help_msg,
                                         true);
        
        update_status("✅ Bootloader reboot initiated...");
    }
}

// Callback: Reboot Recovery
extern "C" void on_reboot_recovery(GtkButton* button, gpointer user_data)
{
    if (!app_state) return;

    auto action = get_reboot_action();
    if (!action || !action->can_reboot()) {
        ui::Dialogs::show_error(GTK_WINDOW(app_state->main_window),
                               "Device Not Connected",
                               "No device is connected. Connect an Android device with USB Debugging enabled.");
        return;
    }

    auto info = action->get_recovery_reboot_info();
    if (ui::Dialogs::confirm_reboot(GTK_WINDOW(app_state->main_window),
                                    info.type, info.description, info.warning)) {
        update_status("Rebooting to recovery...");
        
        std::string cmd = "adb reboot recovery";
        std::string help_msg = "Command sent! If device doesn't reboot, copy and run this in terminal:\n\n";
        help_msg += cmd + "\n\nMake sure USB Debugging is enabled.";
        
        ui::Dialogs::show_command_details(GTK_WINDOW(app_state->main_window),
                                         cmd,
                                         help_msg,
                                         true);
        
        update_status("✅ Recovery reboot initiated...");
    }
}

// Callback: Reboot Download Mode (Samsung)
extern "C" void on_reboot_download(GtkButton* button, gpointer user_data)
{
    if (!app_state) return;

    auto action = get_reboot_action();
    if (!action || !action->can_reboot()) {
        ui::Dialogs::show_error(GTK_WINDOW(app_state->main_window),
                               "Device Not Connected",
                               "No device is connected. Connect an Android device with USB Debugging enabled.");
        return;
    }

    auto info = action->get_download_reboot_info();
    if (ui::Dialogs::confirm_reboot(GTK_WINDOW(app_state->main_window),
                                    info.type, info.description, info.warning)) {
        update_status("Rebooting to Download Mode...");
        
        std::string cmd = "adb reboot download";
        std::string help_msg = "Command sent! If device doesn't reboot, copy and run this in terminal:\n\n";
        help_msg += cmd + "\n\nMake sure USB Debugging is enabled.";
        if (!info.warning.empty()) {
            help_msg += "\n\n⚠️  " + info.warning;
        }
        
        ui::Dialogs::show_command_details(GTK_WINDOW(app_state->main_window),
                                         cmd,
                                         help_msg,
                                         true);
        
        update_status("✅ Download Mode reboot initiated...");
    }
}

// Load and apply CSS styling
static void apply_custom_css(GtkApplication* app)
{
    GtkCssProvider* provider = gtk_css_provider_new();
    const char* css_locations[] = {
        "/usr/share/lincheckroot/style.css",
        "/usr/local/share/lincheckroot/style.css",
        "./data/style.css",
        NULL
    };

    for (size_t i = 0; css_locations[i] != NULL; ++i) {
        if (g_file_test(css_locations[i], G_FILE_TEST_EXISTS)) {
            gtk_css_provider_load_from_path(provider, css_locations[i]);
            gtk_style_context_add_provider_for_display(
                gdk_display_get_default(),
                GTK_STYLE_PROVIDER(provider),
                GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
            break;
        }
    }
    g_object_unref(provider);
}

// Build the GUI
static GtkWidget* build_gui(GtkApplication* app)
{
    // Apply custom CSS styling
    apply_custom_css(app);

    // Main window with modern title bar
    GtkWidget* window = gtk_application_window_new(app);
    gtk_window_set_default_size(GTK_WINDOW(window), 1000, 750);
    gtk_window_set_title(GTK_WINDOW(window), "LinCheckROOT");
    gtk_window_set_icon_name(GTK_WINDOW(window), "lincheckroot");
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);

    app_state->main_window = window;

    // Main container
    GtkWidget* main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_widget_add_css_class(main_box, "main");

    // Header bar with title
    GtkWidget* header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 15);
    gtk_widget_add_css_class(header_box, "header");
    gtk_widget_set_margin_top(header_box, 12);
    gtk_widget_set_margin_bottom(header_box, 12);
    gtk_widget_set_margin_start(header_box, 15);
    gtk_widget_set_margin_end(header_box, 15);

    GtkWidget* title_label = gtk_label_new("Android Device Analyzer");
    gtk_widget_add_css_class(title_label, "title");
    gtk_label_set_xalign(GTK_LABEL(title_label), 0.0);
    gtk_box_append(GTK_BOX(header_box), title_label);
    gtk_box_set_homogeneous(GTK_BOX(header_box), FALSE);
    gtk_box_append(GTK_BOX(main_box), header_box);

    // Separator
    GtkWidget* sep1 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(main_box), sep1);

    // Top control panel - now more organized
    GtkWidget* control_panel = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_widget_add_css_class(control_panel, "section");
    gtk_widget_set_margin_top(control_panel, 12);
    gtk_widget_set_margin_bottom(control_panel, 12);
    gtk_widget_set_margin_start(control_panel, 15);
    gtk_widget_set_margin_end(control_panel, 15);

    // ADB Path row
    GtkWidget* adb_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_set_homogeneous(GTK_BOX(adb_row), FALSE);
    GtkWidget* adb_label = gtk_label_new("ADB Path:");
    gtk_label_set_xalign(GTK_LABEL(adb_label), 0.0);
    gtk_widget_set_size_request(adb_label, 100, -1);
    gtk_box_append(GTK_BOX(adb_row), adb_label);
    
    app_state->adb_path_entry = gtk_entry_new();
    gtk_editable_set_text(GTK_EDITABLE(app_state->adb_path_entry), 
                          app_state->config->get("adb_path").c_str());
    gtk_widget_set_size_request(app_state->adb_path_entry, 350, -1);
    g_signal_connect(app_state->adb_path_entry, "changed", 
                     G_CALLBACK(on_adb_path_changed), nullptr);
    gtk_box_append(GTK_BOX(adb_row), app_state->adb_path_entry);
    gtk_box_append(GTK_BOX(control_panel), adb_row);

    // Devices row
    GtkWidget* device_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_set_homogeneous(GTK_BOX(device_row), FALSE);
    
    GtkWidget* device_label = gtk_label_new("Select Device:");
    gtk_label_set_xalign(GTK_LABEL(device_label), 0.0);
    gtk_widget_set_size_request(device_label, 100, -1);
    gtk_box_append(GTK_BOX(device_row), device_label);

    app_state->device_list_combo = gtk_combo_box_text_new();
    gtk_widget_set_size_request(app_state->device_list_combo, 350, -1);
    g_signal_connect(app_state->device_list_combo, "changed", 
                     G_CALLBACK(on_device_selected), nullptr);
    gtk_box_append(GTK_BOX(device_row), app_state->device_list_combo);

    GtkWidget* scan_button = gtk_button_new_with_label("🔍 Scan Devices");
    gtk_widget_set_size_request(scan_button, 140, -1);
    g_signal_connect(scan_button, "clicked", G_CALLBACK(on_scan_devices), nullptr);
    gtk_box_append(GTK_BOX(device_row), scan_button);

    gtk_box_append(GTK_BOX(control_panel), device_row);
    gtk_box_append(GTK_BOX(main_box), control_panel);

    // Device Actions section
    GtkWidget* device_actions_label = gtk_label_new("");
    gtk_label_set_markup(GTK_LABEL(device_actions_label), "<b>📱 Device Actions (Safe Reboot Only)</b>");
    gtk_label_set_xalign(GTK_LABEL(device_actions_label), 0.0);
    gtk_widget_set_margin_start(device_actions_label, 15);
    gtk_widget_set_margin_top(device_actions_label, 12);
    gtk_box_append(GTK_BOX(main_box), device_actions_label);

    GtkWidget* reboot_buttons_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(reboot_buttons_row, 15);
    gtk_widget_set_margin_end(reboot_buttons_row, 15);
    gtk_widget_set_margin_bottom(reboot_buttons_row, 12);

    GtkWidget* reboot_system_btn = gtk_button_new_with_label("🔄 Reboot System");
    gtk_widget_set_size_request(reboot_system_btn, 150, -1);
    g_signal_connect(reboot_system_btn, "clicked", G_CALLBACK(on_reboot_system), nullptr);
    gtk_box_append(GTK_BOX(reboot_buttons_row), reboot_system_btn);

    GtkWidget* reboot_bootloader_btn = gtk_button_new_with_label("🔧 Bootloader");
    gtk_widget_set_size_request(reboot_bootloader_btn, 150, -1);
    g_signal_connect(reboot_bootloader_btn, "clicked", G_CALLBACK(on_reboot_bootloader), nullptr);
    gtk_box_append(GTK_BOX(reboot_buttons_row), reboot_bootloader_btn);

    GtkWidget* reboot_recovery_btn = gtk_button_new_with_label("⚙️ Recovery");
    gtk_widget_set_size_request(reboot_recovery_btn, 150, -1);
    g_signal_connect(reboot_recovery_btn, "clicked", G_CALLBACK(on_reboot_recovery), nullptr);
    gtk_box_append(GTK_BOX(reboot_buttons_row), reboot_recovery_btn);

    GtkWidget* reboot_download_btn = gtk_button_new_with_label("📥 Download Mode");
    gtk_widget_set_size_request(reboot_download_btn, 150, -1);
    g_signal_connect(reboot_download_btn, "clicked", G_CALLBACK(on_reboot_download), nullptr);
    gtk_box_append(GTK_BOX(reboot_buttons_row), reboot_download_btn);

    gtk_box_append(GTK_BOX(main_box), reboot_buttons_row);

    GtkWidget* warning_label = gtk_label_new("⚠️  All reboots require confirmation. No data will be erased.");
    gtk_label_set_xalign(GTK_LABEL(warning_label), 0.0);
    gtk_widget_set_margin_start(warning_label, 15);
    gtk_widget_set_margin_end(warning_label, 15);
    gtk_widget_set_margin_bottom(warning_label, 12);
    gtk_box_append(GTK_BOX(main_box), warning_label);

    // Action buttons
    GtkWidget* button_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_widget_set_margin_start(button_row, 15);
    gtk_widget_set_margin_end(button_row, 15);
    gtk_widget_set_margin_bottom(button_row, 12);

    GtkWidget* refresh_button = gtk_button_new_with_label("🔄 Refresh Information");
    gtk_widget_set_size_request(refresh_button, 200, -1);
    g_signal_connect(refresh_button, "clicked", G_CALLBACK(on_refresh_info), nullptr);
    gtk_box_append(GTK_BOX(button_row), refresh_button);

    GtkWidget* quit_button = gtk_button_new_with_label("✕ Quit");
    gtk_widget_add_css_class(quit_button, "secondary");
    gtk_widget_set_size_request(quit_button, 100, -1);
    g_signal_connect(quit_button, "clicked", G_CALLBACK(on_quit), nullptr);
    gtk_box_append(GTK_BOX(button_row), quit_button);

    gtk_box_append(GTK_BOX(main_box), button_row);

    // Separator
    GtkWidget* sep2 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(main_box), sep2);

    // Notebook (tabs) - with better spacing
    GtkWidget* notebook = gtk_notebook_new();
    gtk_widget_set_margin_start(notebook, 0);
    gtk_widget_set_margin_end(notebook, 0);
    gtk_widget_set_margin_top(notebook, 0);
    gtk_widget_set_margin_bottom(notebook, 0);

    // Device Info Tab with icon
    app_state->device_info_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app_state->device_info_text), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app_state->device_info_text), GTK_WRAP_WORD);
    gtk_widget_add_css_class(app_state->device_info_text, "textview");

    GtkWidget* device_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(device_scroll), app_state->device_info_text);
    gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(device_scroll), TRUE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), device_scroll, 
                            gtk_label_new("📱 Device Info"));

    // Root Status Tab
    app_state->root_status_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app_state->root_status_text), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app_state->root_status_text), GTK_WRAP_WORD);
    gtk_widget_add_css_class(app_state->root_status_text, "textview");

    GtkWidget* root_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(root_scroll), app_state->root_status_text);
    gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(root_scroll), TRUE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), root_scroll, 
                            gtk_label_new("🔓 Root Status"));

    // Bootloader Status Tab
    app_state->bootloader_status_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app_state->bootloader_status_text), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app_state->bootloader_status_text), GTK_WRAP_WORD);
    gtk_widget_add_css_class(app_state->bootloader_status_text, "textview");

    GtkWidget* bootloader_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(bootloader_scroll), 
                                  app_state->bootloader_status_text);
    gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(bootloader_scroll), TRUE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), bootloader_scroll, 
                            gtk_label_new("🔒 Bootloader"));

    // ROM Compatibility Tab
    app_state->rom_compat_text = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(app_state->rom_compat_text), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(app_state->rom_compat_text), GTK_WRAP_WORD);
    gtk_widget_add_css_class(app_state->rom_compat_text, "textview");

    GtkWidget* rom_scroll = gtk_scrolled_window_new();
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(rom_scroll), app_state->rom_compat_text);
    gtk_scrolled_window_set_has_frame(GTK_SCROLLED_WINDOW(rom_scroll), TRUE);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), rom_scroll, 
                            gtk_label_new("🐧 ROM Compatibility"));

    gtk_box_append(GTK_BOX(main_box), notebook);
    gtk_widget_set_hexpand(notebook, TRUE);
    gtk_widget_set_vexpand(notebook, TRUE);

    // Separator before status bar
    GtkWidget* sep3 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_box_append(GTK_BOX(main_box), sep3);

    // Status bar
    app_state->status_bar = gtk_statusbar_new();
    gtk_widget_add_css_class(app_state->status_bar, "statusbar");
    gtk_statusbar_push(GTK_STATUSBAR(app_state->status_bar), 0, "Ready - Connect device and click 'Scan Devices'");
    gtk_box_append(GTK_BOX(main_box), app_state->status_bar);

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
