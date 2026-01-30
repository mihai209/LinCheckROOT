#ifndef ROOT_ANALYZER_H
#define ROOT_ANALYZER_H

#include "adb_abstraction.h"
#include <string>
#include <optional>

enum class RootStatus {
    ROOTED,
    NOT_ROOTED,
    UNKNOWN
};

enum class RootMethod {
    MAGISK,
    SUPERSU,
    UNKNOWN_METHOD,
    NO_ROOT
};

// Root detection information
struct RootInfo {
    RootStatus status;
    RootMethod method;
    std::string magisk_version;  // empty if not Magisk
    bool has_su = false;
    bool has_magisk_binary = false;
};

// Root Analyzer
// Non-intrusive root detection via standard methods
class RootAnalyzer {
public:
    explicit RootAnalyzer(const AdbAbstraction& adb);

    // Analyze root status
    std::optional<RootInfo> analyze(const std::string& serial) const;

    // Check specific aspects
    bool has_su_binary(const std::string& serial) const;
    bool has_magisk(const std::string& serial) const;
    std::string get_magisk_version(const std::string& serial) const;
    bool has_supersu(const std::string& serial) const;
    
    // Get friendly status string
    std::string status_to_string(RootStatus status) const;
    std::string method_to_string(RootMethod method) const;

private:
    const AdbAbstraction& adb;

    // Check standard su locations
    bool check_su_locations(const std::string& serial) const;

    // Check Magisk-specific markers
    bool check_magisk(const std::string& serial) const;

    // Check SuperSU markers
    bool check_supersu(const std::string& serial) const;
};

#endif // ROOT_ANALYZER_H
