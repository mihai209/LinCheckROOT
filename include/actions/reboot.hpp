#pragma once

#include <string>
#include "adb/adb_client.hpp"
#include "device/device_info.hpp"

namespace actions {

/**
 * Safe reboot action handler
 * All reboots require explicit user confirmation
 * No automatic reboots or data destruction
 */
class RebootAction {
public:
    struct RebootInfo {
        std::string type;           // "system", "bootloader", "recovery", "download"
        std::string description;
        std::string warning;        // Empty if no warning needed
        bool requires_confirmation;
    };

    explicit RebootAction(const adb::AdbClient& adb, const device::DeviceInfo& device);

    // Get info about reboot types
    RebootInfo get_system_reboot_info() const;
    RebootInfo get_bootloader_reboot_info() const;
    RebootInfo get_recovery_reboot_info() const;
    RebootInfo get_download_reboot_info() const;

    // Execute reboots (should only be called after user confirmation)
    bool execute_reboot_system();
    bool execute_reboot_bootloader();
    bool execute_reboot_recovery();
    bool execute_reboot_download();

    // Validation
    bool can_reboot() const;

private:
    const adb::AdbClient& adb_;
    const device::DeviceInfo& device_;
};

} // namespace actions
