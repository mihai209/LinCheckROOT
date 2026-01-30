#include "analyzer/analyzers.hpp"

#include <algorithm>

namespace analyzer {

// ============ SELinuxAnalyzer ============

SELinuxAnalyzer::SELinuxAnalyzer(const adb::AdbClient& adb) 
    : adb_(adb), status_("Unknown") {
    refresh();
}

void SELinuxAnalyzer::refresh() {
    auto result = adb_.shell_command("getenforce");
    if (result) {
        status_ = result.value();
    } else {
        status_ = "Unknown";
    }
}

std::string SELinuxAnalyzer::get_description() const {
    if (status_ == "Enforcing") {
        return "SELinux is enforced (restrictive security)";
    } else if (status_ == "Permissive") {
        return "SELinux is permissive (warnings only)";
    } else if (status_ == "Disabled") {
        return "SELinux is disabled";
    }
    return "Unknown SELinux status";
}

// ============ BootStateAnalyzer ============

BootStateAnalyzer::BootStateAnalyzer(const adb::AdbClient& adb) 
    : adb_(adb), verified_boot_state_("Unknown"), 
      device_state_("Unknown"), vbmeta_state_("Unknown") {
    refresh();
}

void BootStateAnalyzer::refresh() {
    auto vb_state = adb_.shell_getprop("ro.boot.verifiedbootstate");
    verified_boot_state_ = vb_state ? vb_state.value() : "Unknown";

    auto dev_state = adb_.shell_getprop("ro.boot.vbmeta.device_state");
    device_state_ = dev_state ? dev_state.value() : "Unknown";

    // Simplified mapping
    if (device_state_ == "locked") {
        vbmeta_state_ = "Locked";
    } else if (device_state_ == "unlocked") {
        vbmeta_state_ = "Unlocked";
    } else {
        vbmeta_state_ = "Unknown";
    }
}

// ============ OEMUnlockAnalyzer ============

OEMUnlockAnalyzer::OEMUnlockAnalyzer(const adb::AdbClient& adb) 
    : adb_(adb), support_status_("Unknown"), allowed_status_("Unknown") {
    refresh();
}

void OEMUnlockAnalyzer::refresh() {
    auto supported = adb_.shell_getprop("ro.oem_unlock_supported");
    if (supported) {
        support_status_ = (supported.value() == "1") ? "Supported" : "Not Supported";
    } else {
        support_status_ = "Unknown";
    }

    auto allowed = adb_.shell_getprop("sys.oem_unlock_allowed");
    if (allowed) {
        allowed_status_ = (allowed.value() == "1") ? "Allowed" : "Not Allowed";
    } else {
        allowed_status_ = "Unknown";
    }
}

// ============ SlotsAnalyzer ============

SlotsAnalyzer::SlotsAnalyzer(const adb::AdbClient& adb) 
    : adb_(adb), current_slot_("Unknown"), has_ab_(false) {
    refresh();
}

void SlotsAnalyzer::refresh() {
    auto slot_suffix = adb_.shell_getprop("ro.boot.slot_suffix");
    if (slot_suffix) {
        current_slot_ = slot_suffix.value();
        has_ab_ = !current_slot_.empty();
    } else {
        current_slot_ = "Unknown";
        has_ab_ = false;
    }

    // Check for A/B update support
    auto ab_update = adb_.shell_getprop("ro.build.ab_update");
    if (ab_update && ab_update.value() == "true") {
        has_ab_ = true;
    }
}

} // namespace analyzer
