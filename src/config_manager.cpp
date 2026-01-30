#include "config_manager.h"
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <filesystem>
#include <unistd.h>

namespace fs = std::filesystem;

ConfigManager::ConfigManager()
{
    config_file_path = get_config_file();
    load_defaults();
}

std::string ConfigManager::get_config_dir()
{
    const char* home = std::getenv("HOME");
    if (!home) {
        home = "/root";
    }
    return std::string(home) + "/.config/lincheckroot";
}

std::string ConfigManager::get_config_file()
{
    return get_config_dir() + "/config.json";
}

std::string ConfigManager::get_default_adb_path()
{
    return "";  // Empty means auto-detect
}

bool ConfigManager::verify_adb_path(const std::string& path)
{
    if (path.empty()) {
        return true;  // Auto-detect is acceptable
    }
    return access(path.c_str(), X_OK) == 0;
}

void ConfigManager::load_defaults()
{
    config = json::object();
    config["adb_path"] = "";
    config["last_device"] = "";
    config["check_updates"] = false;
}

bool ConfigManager::load()
{
    std::ifstream file(config_file_path);
    if (!file.is_open()) {
        // File doesn't exist, create default
        return save();
    }

    try {
        file >> config;
        return true;
    } catch (...) {
        load_defaults();
        return false;
    }
}

bool ConfigManager::save()
{
    if (!ensure_config_dir()) {
        return false;
    }

    std::ofstream file(config_file_path);
    if (!file.is_open()) {
        return false;
    }

    try {
        file << config.dump(4) << std::endl;
        return true;
    } catch (...) {
        return false;
    }
}

std::string ConfigManager::get(const std::string& key, const std::string& default_value) const
{
    try {
        if (config.contains(key)) {
            return config[key].get<std::string>();
        }
    } catch (...) {}
    return default_value;
}

void ConfigManager::set(const std::string& key, const std::string& value)
{
    config[key] = value;
}

bool ConfigManager::has(const std::string& key) const
{
    return config.contains(key);
}

bool ConfigManager::ensure_config_dir() const
{
    std::string dir = get_config_dir();
    try {
        fs::create_directories(dir);
        return true;
    } catch (...) {
        return false;
    }
}
