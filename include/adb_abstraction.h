#ifndef ADB_ABSTRACTION_H
#define ADB_ABSTRACTION_H

#include <string>
#include <vector>
#include <optional>
#include <map>

// Device states from adb devices
enum class DeviceState {
    DEVICE,           // device (OK)
    UNAUTHORIZED,     // unauthorized
    OFFLINE,          // offline
    UNKNOWN           // unknown state
};

// Structure for detected devices
struct AdbDevice {
    std::string serial;
    DeviceState state;
    std::string state_string;
};

// ADB Abstraction Layer
// Handles all ADB communication with defensive parsing
class AdbAbstraction {
public:
    // Initialize with custom ADB path (empty string = auto-detect from PATH)
    explicit AdbAbstraction(const std::string& custom_adb_path = "");

    // Verify ADB is accessible and working
    bool verify_adb() const;

    // Get ADB executable path
    std::string get_adb_path() const;

    // List connected devices
    std::vector<AdbDevice> list_devices() const;

    // Execute shell command on specific device
    // Returns output or empty string on error
    std::string shell_command(const std::string& serial, const std::string& command) const;

    // Get device property
    std::optional<std::string> get_property(const std::string& serial, const std::string& property) const;

    // Push file to device (for future use)
    bool push_file(const std::string& serial, const std::string& local_path, const std::string& remote_path) const;

    // Pull file from device
    bool pull_file(const std::string& serial, const std::string& remote_path, const std::string& local_path) const;

    // Reboot device (normal/bootloader/recovery)
    bool reboot(const std::string& serial, const std::string& mode = "device") const;

    // Check if device is in fastboot mode
    bool is_fastboot_available() const;

    // Execute fastboot command
    std::string fastboot_command(const std::string& command) const;

private:
    std::string adb_path;
    
    // Execute command and return output
    std::optional<std::string> execute_command(const std::string& command) const;

    // Parse "adb devices" output
    std::vector<AdbDevice> parse_devices_output(const std::string& output) const;

    // Try to find adb in common locations
    std::string find_adb_in_path() const;
};

#endif // ADB_ABSTRACTION_H
