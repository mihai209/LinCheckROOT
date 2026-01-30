#ifndef ROM_COMPATIBILITY_H
#define ROM_COMPATIBILITY_H

#include <string>
#include <vector>
#include <optional>
#include <map>

// ROM compatibility information
struct RomInfo {
    std::string codename;
    bool supported;
    std::string latest_version;
    std::string maintainer;
    std::string url;
    std::vector<std::string> available_versions;
};

struct RomDatabase {
    std::string codename;
    bool is_supported;
    std::string latest_lineage_version;
    std::string maintainer;
    std::string download_url;
    std::vector<std::string> all_versions;
};

// ROM Compatibility Checker
// Offline database lookup for LineageOS compatibility
class RomCompatibility {
public:
    // Initialize with database file path (empty = use default)
    explicit RomCompatibility(const std::string& db_path = "");

    // Check LineageOS compatibility
    std::optional<RomInfo> check_lineage_os(const std::string& codename) const;

    // Load database from JSON file
    bool load_database(const std::string& db_path);

    // Get all supported devices
    std::vector<std::string> get_supported_devices() const;

    // Format ROM info for display
    std::string format_rom_info(const RomInfo& info) const;

private:
    std::map<std::string, RomDatabase> devices_db;
    std::string database_path;

    // Parse JSON database
    bool parse_database_json(const std::string& json_content);
};

#endif // ROM_COMPATIBILITY_H
