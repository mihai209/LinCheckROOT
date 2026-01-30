#include "adb_abstraction.h"
#include <iostream>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <filesystem>
#include <algorithm>

namespace fs = std::filesystem;

AdbAbstraction::AdbAbstraction(const std::string& custom_adb_path)
{
    if (!custom_adb_path.empty()) {
        adb_path = custom_adb_path;
    } else {
        adb_path = find_adb_in_path();
    }
}

std::string AdbAbstraction::find_adb_in_path() const
{
    // Check common locations
    const char* common_paths[] = {
        "/usr/bin/adb",
        "/usr/local/bin/adb",
        "/opt/android-sdk-linux/platform-tools/adb",
        "/home/*/android-sdk/platform-tools/adb",
        "/home/*/.local/share/android-sdk/platform-tools/adb",
    };

    // Try to use "which" command
    FILE* pipe = popen("which adb 2>/dev/null", "r");
    if (pipe) {
        char buffer[128];
        if (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            pclose(pipe);
            std::string result(buffer);
            // Remove newline
            result.erase(std::remove(result.begin(), result.end(), '\n'), result.end());
            if (!result.empty()) {
                return result;
            }
        }
        pclose(pipe);
    }

    // Fallback: check common locations
    for (const auto& path : common_paths) {
        if (access(path, X_OK) == 0) {
            return path;
        }
    }

    return "adb";  // Return "adb" as fallback, will fail on verify
}

bool AdbAbstraction::verify_adb() const
{
    std::string cmd = adb_path + " version >/dev/null 2>&1";
    return system(cmd.c_str()) == 0;
}

std::string AdbAbstraction::get_adb_path() const
{
    return adb_path;
}

std::optional<std::string> AdbAbstraction::execute_command(const std::string& command) const
{
    FILE* pipe = popen(command.c_str(), "r");
    if (!pipe) return std::nullopt;

    std::string result;
    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }

    int status = pclose(pipe);
    if (status != 0) {
        return std::nullopt;
    }

    return result;
}

std::vector<AdbDevice> AdbAbstraction::list_devices() const
{
    std::string cmd = adb_path + " devices";
    auto output = execute_command(cmd);
    
    if (!output) {
        return {};
    }

    return parse_devices_output(output.value());
}

std::vector<AdbDevice> AdbAbstraction::parse_devices_output(const std::string& output) const
{
    std::vector<AdbDevice> devices;
    std::istringstream stream(output);
    std::string line;

    // Skip first line "List of attached devices"
    if (std::getline(stream, line) && line.find("attached devices") == std::string::npos) {
        // Put back first line if it's not the header
        std::istringstream temp_stream(output);
        std::string first_line;
        std::getline(temp_stream, first_line);
    }

    while (std::getline(stream, line)) {
        if (line.empty() || line.find("attached devices") != std::string::npos) {
            continue;
        }

        // Parse: "serial\tstate"
        size_t tab_pos = line.find('\t');
        if (tab_pos == std::string::npos) {
            continue;
        }

        std::string serial = line.substr(0, tab_pos);
        std::string state_str = line.substr(tab_pos + 1);

        // Remove trailing whitespace
        state_str.erase(state_str.find_last_not_of(" \t\n\r") + 1);

        AdbDevice device;
        device.serial = serial;
        device.state_string = state_str;

        if (state_str == "device") {
            device.state = DeviceState::DEVICE;
        } else if (state_str == "unauthorized") {
            device.state = DeviceState::UNAUTHORIZED;
        } else if (state_str == "offline") {
            device.state = DeviceState::OFFLINE;
        } else {
            device.state = DeviceState::UNKNOWN;
        }

        devices.push_back(device);
    }

    return devices;
}

std::string AdbAbstraction::shell_command(const std::string& serial, const std::string& command) const
{
    std::string cmd = adb_path + " -s " + serial + " shell \"" + command + "\" 2>/dev/null";
    auto output = execute_command(cmd);
    return output ? output.value() : "";
}

std::optional<std::string> AdbAbstraction::get_property(const std::string& serial, const std::string& property) const
{
    std::string cmd = "getprop " + property;
    std::string result = shell_command(serial, cmd);
    
    if (result.empty()) {
        return std::nullopt;
    }

    // Remove trailing newline
    result.erase(result.find_last_not_of(" \t\n\r") + 1);
    return result;
}

bool AdbAbstraction::push_file(const std::string& serial, const std::string& local_path, const std::string& remote_path) const
{
    std::string cmd = adb_path + " -s " + serial + " push \"" + local_path + "\" \"" + remote_path + "\" >/dev/null 2>&1";
    return system(cmd.c_str()) == 0;
}

bool AdbAbstraction::pull_file(const std::string& serial, const std::string& remote_path, const std::string& local_path) const
{
    std::string cmd = adb_path + " -s " + serial + " pull \"" + remote_path + "\" \"" + local_path + "\" >/dev/null 2>&1";
    return system(cmd.c_str()) == 0;
}

bool AdbAbstraction::reboot(const std::string& serial, const std::string& mode) const
{
    std::string cmd = adb_path + " -s " + serial + " reboot " + mode + " >/dev/null 2>&1";
    return system(cmd.c_str()) == 0;
}

bool AdbAbstraction::is_fastboot_available() const
{
    return system("which fastboot >/dev/null 2>&1") == 0;
}

std::string AdbAbstraction::fastboot_command(const std::string& command) const
{
    std::string cmd = "fastboot " + command + " 2>/dev/null";
    auto output = execute_command(cmd);
    return output ? output.value() : "";
}
