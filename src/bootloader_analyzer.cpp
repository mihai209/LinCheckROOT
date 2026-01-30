#include "bootloader_analyzer.h"
#include <iostream>
#include <unistd.h>

BootloaderAnalyzer::BootloaderAnalyzer(const AdbAbstraction& adb) : adb(adb) {}

std::optional<BootloaderInfo> BootloaderAnalyzer::analyze(const std::string& serial) const
{
    BootloaderInfo info;
    info.fastboot_path = locate_fastboot();
    info.fastboot_available = !info.fastboot_path.empty();

    // Note: To get actual bootloader status, device needs to be in fastboot mode
    // This requires user to reboot device manually or via ADB
    // For now, we indicate it requires fastboot check

    if (info.fastboot_available) {
        // Could attempt to get status if device is already in fastboot
        // For safety, we'll require user interaction
        info.status = BootloaderStatus::UNKNOWN;
    } else {
        info.status = BootloaderStatus::UNKNOWN;
    }

    info.status_string = status_to_string(info.status);
    return info;
}

BootloaderStatus BootloaderAnalyzer::get_status_via_fastboot() const
{
    // This requires device to be in bootloader/fastboot mode
    std::string output = adb.fastboot_command("getvar unlocked");

    if (output.find("unlocked: yes") != std::string::npos) {
        return BootloaderStatus::UNLOCKED;
    } else if (output.find("unlocked: no") != std::string::npos) {
        return BootloaderStatus::LOCKED;
    }

    return BootloaderStatus::UNKNOWN;
}

std::string BootloaderAnalyzer::status_to_string(BootloaderStatus status) const
{
    switch (status) {
        case BootloaderStatus::LOCKED:
            return "Locked";
        case BootloaderStatus::UNLOCKED:
            return "Unlocked";
        case BootloaderStatus::UNLOCKABLE:
            return "Unlockable (Can be unlocked)";
        case BootloaderStatus::UNKNOWN:
            return "Unknown (Check needed)";
        default:
            return "Unknown";
    }
}

std::string BootloaderAnalyzer::get_data_loss_warning() const
{
    return "⚠️  WARNING: Unlocking bootloader will ERASE all device data!\n"
           "This includes:\n"
           "  • All apps and their data\n"
           "  • Photos, videos, documents\n"
           "  • Contacts and messages\n"
           "  • All personal files\n\n"
           "BACKUP YOUR DATA BEFORE UNLOCKING THE BOOTLOADER!\n"
           "This action is irreversible without restoring a backup.";
}

std::string BootloaderAnalyzer::locate_fastboot() const
{
    // Try which command first
    FILE* pipe = popen("which fastboot 2>/dev/null", "r");
    if (pipe) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            pclose(pipe);
            std::string result(buffer);
            // Remove newline
            result.erase(result.find_last_not_of(" \t\n\r") + 1);
            if (!result.empty()) {
                return result;
            }
        }
        pclose(pipe);
    }

    // Check common locations
    const char* common_paths[] = {
        "/usr/bin/fastboot",
        "/usr/local/bin/fastboot",
        "/opt/android-sdk-linux/platform-tools/fastboot",
    };

    for (const auto& path : common_paths) {
        if (access(path, X_OK) == 0) {
            return path;
        }
    }

    return "";
}

std::string BootloaderAnalyzer::find_fastboot() const
{
    return locate_fastboot();
}
