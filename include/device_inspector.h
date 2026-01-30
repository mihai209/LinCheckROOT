#ifndef DEVICE_INSPECTOR_H
#define DEVICE_INSPECTOR_H

#include "adb_abstraction.h"
#include <string>
#include <map>
#include <optional>

// Device hardware and software information
struct DeviceInfo {
    // Manufacturer and model
    std::string manufacturer;
    std::string model;
    std::string codename;
    
    // Android version info
    std::string android_version;
    int api_level = 0;
    std::string build_fingerprint;
    
    // Hardware
    std::string cpu_abi;
    std::string cpu_abi2;
    int cpu_cores = 0;
    long long ram_mb = 0;
    long long storage_total_mb = 0;
    long long storage_free_mb = 0;
    
    // Kernel
    std::string kernel_version;
    std::string kernel_release;
    
    // Architecture
    std::string arch;
    
    // Build info
    std::string build_type;
    std::string build_date;
    std::string build_id;
    std::string build_host;
};

// Device Inspector
// Extracts comprehensive hardware and software information
class DeviceInspector {
public:
    explicit DeviceInspector(const AdbAbstraction& adb);

    // Inspect device and return full info
    // Returns nullopt if device is not accessible
    std::optional<DeviceInfo> inspect(const std::string& serial) const;

    // Get individual properties safely
    std::string get_manufacturer(const std::string& serial) const;
    std::string get_model(const std::string& serial) const;
    std::string get_codename(const std::string& serial) const;
    std::string get_android_version(const std::string& serial) const;
    int get_api_level(const std::string& serial) const;
    std::string get_build_fingerprint(const std::string& serial) const;
    std::string get_cpu_abi(const std::string& serial) const;
    int get_cpu_cores(const std::string& serial) const;
    long long get_ram_mb(const std::string& serial) const;
    long long get_storage_total_mb(const std::string& serial) const;
    long long get_storage_free_mb(const std::string& serial) const;
    std::string get_kernel_version(const std::string& serial) const;
    std::string get_arch(const std::string& serial) const;

private:
    const AdbAbstraction& adb;

    // Helper: parse /proc/cpuinfo for core count
    int parse_cpu_cores(const std::string& cpuinfo) const;

    // Helper: parse /proc/meminfo for RAM
    long long parse_ram_mb(const std::string& meminfo) const;

    // Helper: extract architecture from ABI
    std::string infer_arch(const std::string& cpu_abi) const;

    // Helper: get storage info from df
    void parse_storage_info(const std::string& df_output, long long& total_mb, long long& free_mb) const;
};

#endif // DEVICE_INSPECTOR_H
