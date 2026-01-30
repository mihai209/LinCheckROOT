#include "device_inspector.h"
#include <sstream>
#include <cctype>
#include <algorithm>

DeviceInspector::DeviceInspector(const AdbAbstraction& adb) : adb(adb) {}

std::optional<DeviceInfo> DeviceInspector::inspect(const std::string& serial) const
{
    DeviceInfo info;

    // Manufacturer and model
    auto mfg = adb.get_property(serial, "ro.product.manufacturer");
    auto model = adb.get_property(serial, "ro.product.model");
    auto codename = adb.get_property(serial, "ro.product.device");

    if (!mfg || !model) {
        return std::nullopt;  // Device not accessible
    }

    info.manufacturer = mfg.value();
    info.model = model.value();
    info.codename = codename ? codename.value() : "unknown";

    // Android version
    auto android_ver = adb.get_property(serial, "ro.build.version.release");
    auto api_level = adb.get_property(serial, "ro.build.version.sdk");

    if (android_ver) {
        info.android_version = android_ver.value();
    }
    if (api_level) {
        try {
            info.api_level = std::stoi(api_level.value());
        } catch (...) {}
    }

    // Build fingerprint
    auto fingerprint = adb.get_property(serial, "ro.build.fingerprint");
    if (fingerprint) {
        info.build_fingerprint = fingerprint.value();
    }

    // CPU ABI
    auto cpu_abi = adb.get_property(serial, "ro.product.cpu.abi");
    auto cpu_abi2 = adb.get_property(serial, "ro.product.cpu.abi2");

    if (cpu_abi) {
        info.cpu_abi = cpu_abi.value();
        info.arch = infer_arch(info.cpu_abi);
    }
    if (cpu_abi2) {
        info.cpu_abi2 = cpu_abi2.value();
    }

    // CPU cores
    std::string cpuinfo = adb.shell_command(serial, "cat /proc/cpuinfo");
    info.cpu_cores = parse_cpu_cores(cpuinfo);

    // RAM
    std::string meminfo = adb.shell_command(serial, "cat /proc/meminfo");
    info.ram_mb = parse_ram_mb(meminfo);

    // Storage
    std::string df_output = adb.shell_command(serial, "df /data");
    parse_storage_info(df_output, info.storage_total_mb, info.storage_free_mb);

    // Kernel
    auto kernel_release = adb.get_property(serial, "ro.build.version.release");
    auto kernel_version = adb.get_property(serial, "ro.kernel.version");

    if (kernel_version) {
        info.kernel_version = kernel_version.value();
    }

    // Build info
    auto build_type = adb.get_property(serial, "ro.build.type");
    auto build_date = adb.get_property(serial, "ro.build.date.utc");
    auto build_id = adb.get_property(serial, "ro.build.id");
    auto build_host = adb.get_property(serial, "ro.build.host");

    if (build_type) info.build_type = build_type.value();
    if (build_date) info.build_date = build_date.value();
    if (build_id) info.build_id = build_id.value();
    if (build_host) info.build_host = build_host.value();

    return info;
}

std::string DeviceInspector::get_manufacturer(const std::string& serial) const
{
    auto val = adb.get_property(serial, "ro.product.manufacturer");
    return val ? val.value() : "Unknown";
}

std::string DeviceInspector::get_model(const std::string& serial) const
{
    auto val = adb.get_property(serial, "ro.product.model");
    return val ? val.value() : "Unknown";
}

std::string DeviceInspector::get_codename(const std::string& serial) const
{
    auto val = adb.get_property(serial, "ro.product.device");
    return val ? val.value() : "Unknown";
}

std::string DeviceInspector::get_android_version(const std::string& serial) const
{
    auto val = adb.get_property(serial, "ro.build.version.release");
    return val ? val.value() : "Unknown";
}

int DeviceInspector::get_api_level(const std::string& serial) const
{
    auto val = adb.get_property(serial, "ro.build.version.sdk");
    if (val) {
        try {
            return std::stoi(val.value());
        } catch (...) {}
    }
    return 0;
}

std::string DeviceInspector::get_build_fingerprint(const std::string& serial) const
{
    auto val = adb.get_property(serial, "ro.build.fingerprint");
    return val ? val.value() : "Unknown";
}

std::string DeviceInspector::get_cpu_abi(const std::string& serial) const
{
    auto val = adb.get_property(serial, "ro.product.cpu.abi");
    return val ? val.value() : "Unknown";
}

int DeviceInspector::get_cpu_cores(const std::string& serial) const
{
    std::string cpuinfo = adb.shell_command(serial, "cat /proc/cpuinfo");
    return parse_cpu_cores(cpuinfo);
}

long long DeviceInspector::get_ram_mb(const std::string& serial) const
{
    std::string meminfo = adb.shell_command(serial, "cat /proc/meminfo");
    return parse_ram_mb(meminfo);
}

long long DeviceInspector::get_storage_total_mb(const std::string& serial) const
{
    std::string df_output = adb.shell_command(serial, "df /data");
    long long total = 0, free = 0;
    parse_storage_info(df_output, total, free);
    return total;
}

long long DeviceInspector::get_storage_free_mb(const std::string& serial) const
{
    std::string df_output = adb.shell_command(serial, "df /data");
    long long total = 0, free = 0;
    parse_storage_info(df_output, total, free);
    return free;
}

std::string DeviceInspector::get_kernel_version(const std::string& serial) const
{
    auto val = adb.get_property(serial, "ro.kernel.version");
    return val ? val.value() : "Unknown";
}

std::string DeviceInspector::get_arch(const std::string& serial) const
{
    auto abi = get_cpu_abi(serial);
    return infer_arch(abi);
}

int DeviceInspector::parse_cpu_cores(const std::string& cpuinfo) const
{
    int cores = 0;
    std::istringstream stream(cpuinfo);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.find("processor") == 0) {
            cores++;
        }
    }

    return cores > 0 ? cores : 1;
}

long long DeviceInspector::parse_ram_mb(const std::string& meminfo) const
{
    std::istringstream stream(meminfo);
    std::string line;

    while (std::getline(stream, line)) {
        if (line.find("MemTotal") == 0) {
            std::istringstream value_stream(line);
            std::string key;
            long long kb;
            value_stream >> key >> kb;
            return kb / 1024;
        }
    }

    return 0;
}

std::string DeviceInspector::infer_arch(const std::string& cpu_abi) const
{
    if (cpu_abi.find("arm64") != std::string::npos) {
        return "ARM64";
    } else if (cpu_abi.find("armeabi") != std::string::npos) {
        return "ARM32";
    } else if (cpu_abi.find("x86_64") != std::string::npos) {
        return "x86_64";
    } else if (cpu_abi.find("x86") != std::string::npos) {
        return "x86";
    }
    return "Unknown";
}

void DeviceInspector::parse_storage_info(const std::string& df_output, long long& total_mb, long long& free_mb) const
{
    std::istringstream stream(df_output);
    std::string line;

    // Skip header
    std::getline(stream, line);

    if (std::getline(stream, line)) {
        std::istringstream line_stream(line);
        std::string filesystem;
        long long blocks, used, available;

        line_stream >> filesystem >> blocks >> used >> available;

        // Parse in blocks (usually 1KB blocks)
        total_mb = blocks / 1024;
        free_mb = available / 1024;
    }
}
