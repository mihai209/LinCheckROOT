#pragma once

#include <string>
#include <map>
#include "adb/adb_client.hpp"

namespace device {

/**
 * Device information aggregator
 * Collects and caches device properties
 */
class DeviceInfo {
public:
    explicit DeviceInfo(const adb::AdbClient& adb);

    // Basic info
    std::string manufacturer() const { return manufacturer_; }
    std::string model() const { return model_; }
    std::string android_version() const { return android_version_; }
    std::string build_fingerprint() const { return build_fingerprint_; }

    // Safe queries
    bool is_samsung() const;
    bool is_connected() const;
    std::string serial() const { return serial_; }

    // Refresh all properties
    void refresh();

private:
    const adb::AdbClient& adb_;

    std::string manufacturer_;
    std::string model_;
    std::string android_version_;
    std::string build_fingerprint_;
    std::string serial_;
};

} // namespace device
