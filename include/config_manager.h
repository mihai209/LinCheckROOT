#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <string>
#include <map>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Configuration manager
// Handles ~/.config/lincheckroot/config.json
class ConfigManager {
public:
    ConfigManager();

    // Load configuration from file
    bool load();

    // Save configuration to file
    bool save();

    // Get configuration value
    std::string get(const std::string& key, const std::string& default_value = "") const;

    // Set configuration value
    void set(const std::string& key, const std::string& value);

    // Check if key exists
    bool has(const std::string& key) const;

    // Get config directory path
    static std::string get_config_dir();

    // Get config file path
    static std::string get_config_file();

    // Get default ADB path
    static std::string get_default_adb_path();

    // Verify ADB path is valid
    static bool verify_adb_path(const std::string& path);

private:
    json config;
    std::string config_file_path;

    // Create config directory if needed
    bool ensure_config_dir() const;

    // Load default configuration
    void load_defaults();
};

#endif // CONFIG_MANAGER_H
