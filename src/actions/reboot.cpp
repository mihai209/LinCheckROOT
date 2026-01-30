#include "actions/reboot.hpp"

namespace actions {

RebootAction::RebootAction(const adb::AdbClient& adb, const device::DeviceInfo& device)
    : adb_(adb), device_(device) {}

bool RebootAction::can_reboot() const {
    return device_.is_connected();
}

RebootAction::RebootInfo RebootAction::get_system_reboot_info() const {
    return {
        "system",
        "Reboot device to Android system",
        "",  // No warning
        true
    };
}

RebootAction::RebootInfo RebootAction::get_bootloader_reboot_info() const {
    std::string warning;
    if (device_.is_samsung()) {
        warning = "Samsung devices do not expose standard fastboot.\n"
                  "Device will enter Download Mode instead.";
    }

    return {
        "bootloader",
        "Reboot to bootloader/Download Mode",
        warning,
        true
    };
}

RebootAction::RebootInfo RebootAction::get_recovery_reboot_info() const {
    return {
        "recovery",
        "Reboot to recovery mode",
        "",  // No warning
        true
    };
}

RebootAction::RebootInfo RebootAction::get_download_reboot_info() const {
    std::string warning;
    if (!device_.is_samsung()) {
        warning = "Download Mode is specific to Samsung devices.\n"
                  "This command may not work on your device.";
    }

    return {
        "download",
        "Reboot to Download Mode (Samsung)",
        warning,
        true
    };
}

bool RebootAction::execute_reboot_system() {
    if (!can_reboot()) return false;
    return adb_.reboot_system();
}

bool RebootAction::execute_reboot_bootloader() {
    if (!can_reboot()) return false;
    return adb_.reboot_bootloader();
}

bool RebootAction::execute_reboot_recovery() {
    if (!can_reboot()) return false;
    return adb_.reboot_recovery();
}

bool RebootAction::execute_reboot_download() {
    if (!can_reboot()) return false;
    return adb_.reboot_download_mode();
}

} // namespace actions
