#include "adb/adb_client.hpp"

#include <cstdlib>
#include <cstdio>
#include <sstream>
#include <algorithm>
#include <cstring>
#include <unistd.h>
#include <sys/stat.h>

namespace adb {

AdbClient::AdbClient(const std::string& adb_path) : adb_path_(adb_path) {}

bool AdbClient::validate_adb_path() const {
    struct stat buffer;
    return (stat(adb_path_.c_str(), &buffer) == 0);
}

std::optional<std::string> AdbClient::execute_command(const std::string& cmd) const {
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe) return std::nullopt;

    std::string result;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);

    return result;
}

std::string AdbClient::trim(const std::string& str) const {
    auto start = str.begin();
    while (start != str.end() && std::isspace(*start)) {
        start++;
    }

    auto end = str.end();
    do {
        end--;
    } while (std::distance(start, end) > 0 && std::isspace(*end));

    return std::string(start, end + 1);
}

bool AdbClient::is_device_connected() const {
    if (!validate_adb_path()) return false;

    std::string cmd = adb_path_ + " get-state";
    auto result = execute_command(cmd);
    if (!result) return false;

    std::string state = trim(result.value());
    return state == "device";
}

std::string AdbClient::get_device_state() const {
    if (!validate_adb_path()) return "unknown";

    std::string cmd = adb_path_ + " get-state";
    auto result = execute_command(cmd);
    if (!result) return "unknown";

    return trim(result.value());
}

std::string AdbClient::get_device_serial() const {
    if (!validate_adb_path()) return "";

    std::string cmd = adb_path_ + " get-serialno";
    auto result = execute_command(cmd);
    if (!result) return "";

    return trim(result.value());
}

std::optional<std::string> AdbClient::shell_getprop(const std::string& prop) const {
    if (!is_device_connected()) return std::nullopt;

    std::string cmd = adb_path_ + " shell getprop " + prop;
    auto result = execute_command(cmd);
    if (!result) return std::nullopt;

    return trim(result.value());
}

std::optional<std::string> AdbClient::shell_command(const std::string& cmd) const {
    if (!is_device_connected()) return std::nullopt;

    std::string full_cmd = adb_path_ + " shell " + cmd;
    auto result = execute_command(full_cmd);
    if (!result) return std::nullopt;

    return trim(result.value());
}

std::string AdbClient::get_device_model() const {
    auto model = shell_getprop("ro.product.model");
    return model ? model.value() : "Unknown";
}

std::string AdbClient::get_device_manufacturer() const {
    auto mfg = shell_getprop("ro.product.manufacturer");
    return mfg ? mfg.value() : "Unknown";
}

std::string AdbClient::get_android_version() const {
    auto version = shell_getprop("ro.build.version.release");
    return version ? version.value() : "Unknown";
}

std::string AdbClient::get_build_fingerprint() const {
    auto fingerprint = shell_getprop("ro.build.fingerprint");
    return fingerprint ? fingerprint.value() : "Unknown";
}

bool AdbClient::reboot_system() const {
    if (!is_device_connected()) return false;

    std::string cmd = adb_path_ + " reboot 2>&1";
    auto result = execute_command(cmd);
    
    if (!result.has_value()) return false;
    
    // Check for error messages
    std::string output = trim(result.value());
    if (output.find("error") != std::string::npos || 
        output.find("failed") != std::string::npos) {
        return false;
    }
    
    // Command executed, wait for device to disconnect
    usleep(500000);  // 500ms delay
    return true;
}

bool AdbClient::reboot_bootloader() const {
    if (!is_device_connected()) return false;

    std::string cmd = adb_path_ + " reboot bootloader 2>&1";
    auto result = execute_command(cmd);
    
    if (!result.has_value()) return false;
    
    std::string output = trim(result.value());
    if (output.find("error") != std::string::npos || 
        output.find("failed") != std::string::npos) {
        return false;
    }
    
    usleep(500000);  // 500ms delay
    return true;
}

bool AdbClient::reboot_recovery() const {
    if (!is_device_connected()) return false;

    std::string cmd = adb_path_ + " reboot recovery 2>&1";
    auto result = execute_command(cmd);
    
    if (!result.has_value()) return false;
    
    std::string output = trim(result.value());
    if (output.find("error") != std::string::npos || 
        output.find("failed") != std::string::npos) {
        return false;
    }
    
    usleep(500000);  // 500ms delay
    return true;
}

bool AdbClient::reboot_download_mode() const {
    if (!is_device_connected()) return false;

    std::string cmd = adb_path_ + " reboot download 2>&1";
    auto result = execute_command(cmd);
    
    if (!result.has_value()) return false;
    
    std::string output = trim(result.value());
    if (output.find("error") != std::string::npos || 
        output.find("failed") != std::string::npos) {
        return false;
    }
    
    usleep(500000);  // 500ms delay
    return true;
}

// Detailed versions that return command and output
AdbClient::CommandResult AdbClient::reboot_system_detailed() const {
    CommandResult result;
    result.command = adb_path_ + " reboot";
    
    if (!is_device_connected()) {
        result.output = "Error: Device not connected";
        result.success = false;
        return result;
    }

    std::string cmd = adb_path_ + " reboot 2>&1";
    auto output = execute_command(cmd);
    
    if (!output.has_value()) {
        result.output = "Error: Failed to execute command";
        result.success = false;
    } else {
        result.output = trim(output.value());
        result.success = result.output.find("error") == std::string::npos && 
                        result.output.find("failed") == std::string::npos;
        if (result.output.empty()) {
            result.output = "Command sent successfully (no output)";
        }
    }
    
    usleep(500000);
    return result;
}

AdbClient::CommandResult AdbClient::reboot_bootloader_detailed() const {
    CommandResult result;
    result.command = adb_path_ + " reboot bootloader";
    
    if (!is_device_connected()) {
        result.output = "Error: Device not connected";
        result.success = false;
        return result;
    }

    std::string cmd = adb_path_ + " reboot bootloader 2>&1";
    auto output = execute_command(cmd);
    
    if (!output.has_value()) {
        result.output = "Error: Failed to execute command";
        result.success = false;
    } else {
        result.output = trim(output.value());
        result.success = result.output.find("error") == std::string::npos && 
                        result.output.find("failed") == std::string::npos;
        if (result.output.empty()) {
            result.output = "Command sent successfully (no output)";
        }
    }
    
    usleep(500000);
    return result;
}

AdbClient::CommandResult AdbClient::reboot_recovery_detailed() const {
    CommandResult result;
    result.command = adb_path_ + " reboot recovery";
    
    if (!is_device_connected()) {
        result.output = "Error: Device not connected";
        result.success = false;
        return result;
    }

    std::string cmd = adb_path_ + " reboot recovery 2>&1";
    auto output = execute_command(cmd);
    
    if (!output.has_value()) {
        result.output = "Error: Failed to execute command";
        result.success = false;
    } else {
        result.output = trim(output.value());
        result.success = result.output.find("error") == std::string::npos && 
                        result.output.find("failed") == std::string::npos;
        if (result.output.empty()) {
            result.output = "Command sent successfully (no output)";
        }
    }
    
    usleep(500000);
    return result;
}

AdbClient::CommandResult AdbClient::reboot_download_mode_detailed() const {
    CommandResult result;
    result.command = adb_path_ + " reboot download";
    
    if (!is_device_connected()) {
        result.output = "Error: Device not connected";
        result.success = false;
        return result;
    }

    std::string cmd = adb_path_ + " reboot download 2>&1";
    auto output = execute_command(cmd);
    
    if (!output.has_value()) {
        result.output = "Error: Failed to execute command";
        result.success = false;
    } else {
        result.output = trim(output.value());
        result.success = result.output.find("error") == std::string::npos && 
                        result.output.find("failed") == std::string::npos;
        if (result.output.empty()) {
            result.output = "Command sent successfully (no output)";
        }
    }
    
    usleep(500000);
    return result;
}

} // namespace adb
