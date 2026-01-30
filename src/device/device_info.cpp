#include "device/device_info.hpp"

#include <algorithm>

namespace device {

DeviceInfo::DeviceInfo(const adb::AdbClient& adb) : adb_(adb) {
    refresh();
}

void DeviceInfo::refresh() {
    manufacturer_ = adb_.get_device_manufacturer();
    model_ = adb_.get_device_model();
    android_version_ = adb_.get_android_version();
    build_fingerprint_ = adb_.get_build_fingerprint();
    serial_ = adb_.get_device_serial();
}

bool DeviceInfo::is_samsung() const {
    std::string lower_mfg = manufacturer_;
    std::transform(lower_mfg.begin(), lower_mfg.end(), lower_mfg.begin(), ::tolower);
    return lower_mfg.find("samsung") != std::string::npos;
}

bool DeviceInfo::is_connected() const {
    return !serial_.empty() && serial_ != "?";
}

} // namespace device
