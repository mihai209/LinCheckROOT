#pragma once

#include <string>
#include <gtk/gtk.h>

namespace ui {

/**
 * Confirmation and warning dialogs
 * Used for user confirmations before reboot operations
 */
class Dialogs {
public:
    /**
     * Show confirmation dialog for reboot operations
     * Returns: true if user clicked OK, false otherwise
     */
    static bool confirm_reboot(GtkWindow* parent,
                              const std::string& reboot_type,
                              const std::string& description,
                              const std::string& warning = "");

    /**
     * Show warning dialog with informational message
     */
    static void show_warning(GtkWindow* parent,
                            const std::string& title,
                            const std::string& message);

    /**
     * Show error dialog
     */
    static void show_error(GtkWindow* parent,
                          const std::string& title,
                          const std::string& message);

    /**
     * Show info dialog
     */
    static void show_info(GtkWindow* parent,
                         const std::string& title,
                         const std::string& message);

    /**
     * Show command execution dialog with command details and copy button
     */
    static void show_command_details(GtkWindow* parent,
                                    const std::string& command,
                                    const std::string& output,
                                    bool success);

private:
    Dialogs() = default;
};

} // namespace ui
