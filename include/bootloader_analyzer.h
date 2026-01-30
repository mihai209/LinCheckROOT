#ifndef BOOTLOADER_ANALYZER_H
#define BOOTLOADER_ANALYZER_H

#include "adb_abstraction.h"
#include <string>
#include <optional>

enum class BootloaderStatus {
    LOCKED,
    UNLOCKED,
    UNLOCKABLE,     // Locked but can be unlocked (usually)
    UNKNOWN
};

// Bootloader information
struct BootloaderInfo {
    BootloaderStatus status;
    std::string status_string;
    bool fastboot_available;
    std::string fastboot_path;
};

// Bootloader Analyzer
// Detects bootloader lock status with clear warnings
class BootloaderAnalyzer {
public:
    explicit BootloaderAnalyzer(const AdbAbstraction& adb);

    // Analyze bootloader status
    // Note: May require device reboot to bootloader mode
    std::optional<BootloaderInfo> analyze(const std::string& serial) const;

    // Get bootloader status from fastboot
    BootloaderStatus get_status_via_fastboot() const;

    // Get friendly status string
    std::string status_to_string(BootloaderStatus status) const;

    // Get warning about data loss
    std::string get_data_loss_warning() const;

    // Find fastboot executable
    std::string find_fastboot() const;

private:
    const AdbAbstraction& adb;

    // Check common fastboot locations
    std::string locate_fastboot() const;
};

#endif // BOOTLOADER_ANALYZER_H
