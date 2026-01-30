#pragma once

#include <string>
#include <vector>
#include <optional>
#include <cstdint>

namespace adb {

/**
 * Safe ADB command executor
 * All operations are read-only or explicitly safe
 * No destructive operations (flash, format, wipe)
 */
class AdbClient {
public:
    AdbClient(const std::string& adb_path = "adb");
    ~AdbClient() = default;

    // Device state checks
    bool is_device_connected() const;
    std::string get_device_state() const;
    std::string get_device_serial() const;

    // Safe shell command execution (read-only)
    std::optional<std::string> shell_getprop(const std::string& prop) const;
    std::optional<std::string> shell_command(const std::string& cmd) const;

    // Device info queries
    std::string get_device_model() const;
    std::string get_device_manufacturer() const;
    std::string get_android_version() const;
    std::string get_build_fingerprint() const;

    // Safe reboot operations (require explicit user confirmation)
    bool reboot_system() const;           // adb reboot
    bool reboot_bootloader() const;       // adb reboot bootloader
    bool reboot_recovery() const;         // adb reboot recovery
    bool reboot_download_mode() const;    // adb reboot download (Samsung)

    // Validation
    bool validate_adb_path() const;
    std::string get_adb_path() const { return adb_path_; }

private:
    std::string adb_path_;

    // Execute raw command and return stdout
    std::optional<std::string> execute_command(const std::string& cmd) const;

    // Parse command output safely
    std::string trim(const std::string& str) const;
};

} // namespace adb
