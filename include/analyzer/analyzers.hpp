#pragma once

#include <string>
#include "adb/adb_client.hpp"

namespace analyzer {

/**
 * SELinux status analyzer (read-only)
 */
class SELinuxAnalyzer {
public:
    explicit SELinuxAnalyzer(const adb::AdbClient& adb);

    std::string get_status() const { return status_; }
    std::string get_description() const;

    void refresh();

private:
    const adb::AdbClient& adb_;
    std::string status_;
};

/**
 * Boot state analyzer - Verified Boot & AVB (read-only)
 */
class BootStateAnalyzer {
public:
    explicit BootStateAnalyzer(const adb::AdbClient& adb);

    std::string get_verified_boot_state() const { return verified_boot_state_; }
    std::string get_device_state() const { return device_state_; }
    std::string get_vbmeta_state() const { return vbmeta_state_; }

    void refresh();

private:
    const adb::AdbClient& adb_;
    std::string verified_boot_state_;
    std::string device_state_;
    std::string vbmeta_state_;
};

/**
 * OEM unlock analyzer (read-only)
 */
class OEMUnlockAnalyzer {
public:
    explicit OEMUnlockAnalyzer(const adb::AdbClient& adb);

    std::string get_support_status() const { return support_status_; }
    std::string get_allowed_status() const { return allowed_status_; }

    void refresh();

private:
    const adb::AdbClient& adb_;
    std::string support_status_;
    std::string allowed_status_;
};

/**
 * A/B slots analyzer (read-only)
 */
class SlotsAnalyzer {
public:
    explicit SlotsAnalyzer(const adb::AdbClient& adb);

    std::string get_current_slot() const { return current_slot_; }
    bool has_ab_partitions() const { return has_ab_; }

    void refresh();

private:
    const adb::AdbClient& adb_;
    std::string current_slot_;
    bool has_ab_;
};

} // namespace analyzer
