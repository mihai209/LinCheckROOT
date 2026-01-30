#include "root_analyzer.h"

RootAnalyzer::RootAnalyzer(const AdbAbstraction& adb) : adb(adb) {}

std::optional<RootInfo> RootAnalyzer::analyze(const std::string& serial) const
{
    RootInfo info;
    info.status = RootStatus::NOT_ROOTED;
    info.method = RootMethod::NO_ROOT;

    // Check for su binary
    info.has_su = check_su_locations(serial);

    // Check for Magisk
    info.has_magisk_binary = check_magisk(serial);
    if (info.has_magisk_binary) {
        info.magisk_version = get_magisk_version(serial);
        info.method = RootMethod::MAGISK;
        info.status = RootStatus::ROOTED;
    }

    // Check for SuperSU
    if (!info.has_magisk_binary && check_supersu(serial)) {
        info.method = RootMethod::SUPERSU;
        info.status = RootStatus::ROOTED;
    }

    // If su exists but no known root method detected
    if (info.has_su && info.method == RootMethod::NO_ROOT) {
        info.method = RootMethod::UNKNOWN_METHOD;
        info.status = RootStatus::ROOTED;
    }

    // If no root indicators found
    if (info.status == RootStatus::NOT_ROOTED) {
        info.method = RootMethod::NO_ROOT;
    }

    return info;
}

bool RootAnalyzer::check_su_locations(const std::string& serial) const
{
    // Common su locations
    const char* su_locations[] = {
        "/system/bin/su",
        "/system/xbin/su",
        "/sbin/su",
        "/bin/su",
        "/system/app/SuperSU/SuperSU.apk",
        "/system/app/Superuser/Superuser.apk",
    };

    // Use which to find su
    std::string output = adb.shell_command(serial, "which su 2>/dev/null");
    if (!output.empty()) {
        return true;
    }

    // Check standard locations
    for (const auto& loc : su_locations) {
        std::string cmd = "test -f " + std::string(loc) + " && echo found";
        std::string result = adb.shell_command(serial, cmd);
        if (result.find("found") != std::string::npos) {
            return true;
        }
    }

    return false;
}

bool RootAnalyzer::check_magisk(const std::string& serial) const
{
    // Check Magisk binary
    std::string result = adb.shell_command(serial, "which magisk 2>/dev/null");
    if (!result.empty() && result.find("magisk") != std::string::npos) {
        return true;
    }

    // Check for Magisk markers
    result = adb.shell_command(serial, "test -d /sbin/.magisk && echo found");
    if (result.find("found") != std::string::npos) {
        return true;
    }

    // Check for Magisk in /data/adb
    result = adb.shell_command(serial, "test -d /data/adb/modules && echo found");
    if (result.find("found") != std::string::npos) {
        return true;
    }

    return false;
}

bool RootAnalyzer::check_supersu(const std::string& serial) const
{
    // Check for SuperSU app
    std::string result = adb.shell_command(serial, "pm list packages | grep -i supersu");
    if (!result.empty()) {
        return true;
    }

    // Check for SuperSU binary
    result = adb.shell_command(serial, "test -f /system/app/SuperSU.apk && echo found");
    if (result.find("found") != std::string::npos) {
        return true;
    }

    return false;
}

bool RootAnalyzer::has_su_binary(const std::string& serial) const
{
    return check_su_locations(serial);
}

bool RootAnalyzer::has_magisk(const std::string& serial) const
{
    return check_magisk(serial);
}

std::string RootAnalyzer::get_magisk_version(const std::string& serial) const
{
    std::string result = adb.shell_command(serial, "magisk --version 2>/dev/null");
    if (!result.empty()) {
        // Remove newline
        result.erase(result.find_last_not_of(" \t\n\r") + 1);
        return result;
    }
    return "Unknown";
}

bool RootAnalyzer::has_supersu(const std::string& serial) const
{
    return check_supersu(serial);
}

std::string RootAnalyzer::status_to_string(RootStatus status) const
{
    switch (status) {
        case RootStatus::ROOTED:
            return "Rooted";
        case RootStatus::NOT_ROOTED:
            return "Not Rooted";
        case RootStatus::UNKNOWN:
            return "Unknown";
        default:
            return "Unknown";
    }
}

std::string RootAnalyzer::method_to_string(RootMethod method) const
{
    switch (method) {
        case RootMethod::MAGISK:
            return "Magisk";
        case RootMethod::SUPERSU:
            return "SuperSU";
        case RootMethod::UNKNOWN_METHOD:
            return "Unknown Method";
        case RootMethod::NO_ROOT:
            return "No Root";
        default:
            return "Unknown";
    }
}
