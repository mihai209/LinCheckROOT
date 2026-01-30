#include "rom_compatibility.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <sstream>
#include <filesystem>

using json = nlohmann::json;
namespace fs = std::filesystem;

RomCompatibility::RomCompatibility(const std::string& db_path)
{
    if (!db_path.empty()) {
        load_database(db_path);
    }
}

bool RomCompatibility::load_database(const std::string& db_path)
{
    database_path = db_path;
    std::ifstream file(db_path);

    if (!file.is_open()) {
        return false;
    }

    try {
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        return parse_database_json(content);
    } catch (...) {
        return false;
    }
}

bool RomCompatibility::parse_database_json(const std::string& json_content)
{
    try {
        auto db_json = json::parse(json_content);

        // Database structure:
        // {
        //   "devices": [
        //     {
        //       "codename": "hammerhead",
        //       "is_supported": true,
        //       "latest_lineage_version": "19.1",
        //       "maintainer": "John Doe",
        //       "download_url": "https://...",
        //       "all_versions": ["19.1", "19", "18.1", ...]
        //     },
        //     ...
        //   ]
        // }

        if (!db_json.contains("devices")) {
            return false;
        }

        devices_db.clear();

        for (const auto& device_json : db_json["devices"]) {
            RomDatabase device;
            device.codename = device_json.value("codename", "");
            device.is_supported = device_json.value("is_supported", false);
            device.latest_lineage_version = device_json.value("latest_lineage_version", "");
            device.maintainer = device_json.value("maintainer", "");
            device.download_url = device_json.value("download_url", "");

            if (device_json.contains("all_versions")) {
                device.all_versions = device_json["all_versions"].get<std::vector<std::string>>();
            }

            devices_db[device.codename] = device;
        }

        return true;
    } catch (...) {
        return false;
    }
}

std::optional<RomInfo> RomCompatibility::check_lineage_os(const std::string& codename) const
{
    auto it = devices_db.find(codename);
    if (it == devices_db.end()) {
        return std::nullopt;  // Device not in database
    }

    const auto& db_entry = it->second;

    RomInfo info;
    info.codename = codename;
    info.supported = db_entry.is_supported;
    info.latest_version = db_entry.latest_lineage_version;
    info.maintainer = db_entry.maintainer;
    info.url = db_entry.download_url;
    info.available_versions = db_entry.all_versions;

    return info;
}

std::vector<std::string> RomCompatibility::get_supported_devices() const
{
    std::vector<std::string> supported;
    for (const auto& [codename, device] : devices_db) {
        if (device.is_supported) {
            supported.push_back(codename);
        }
    }
    return supported;
}

std::string RomCompatibility::format_rom_info(const RomInfo& info) const
{
    std::stringstream ss;

    ss << "Device: " << info.codename << "\n";
    ss << "Status: " << (info.supported ? "Supported" : "Not Supported") << "\n";

    if (info.supported) {
        ss << "Latest Version: " << info.latest_version << "\n";
        ss << "Maintainer: " << info.maintainer << "\n";
        ss << "Download: " << info.url << "\n";

        if (!info.available_versions.empty()) {
            ss << "Available Versions: ";
            for (size_t i = 0; i < info.available_versions.size(); ++i) {
                ss << info.available_versions[i];
                if (i < info.available_versions.size() - 1) {
                    ss << ", ";
                }
            }
            ss << "\n";
        }
    }

    return ss.str();
}
