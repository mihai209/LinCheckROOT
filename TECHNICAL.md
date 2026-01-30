# LinCheckROOT Technical Documentation

## Architecture Overview

LinCheckROOT is organized into modular, independent components that communicate through well-defined interfaces. The separation of concerns ensures code auditability and maintainability.

### Core Modules

#### 1. ADB Abstraction Layer (`adb_abstraction.cpp/.h`)

**Purpose**: Encapsulate all direct ADB communication with defensive parsing.

**Key Features**:
- Auto-detection of ADB in PATH or common locations
- Support for custom ADB paths via configuration
- Defensive parsing of ADB command outputs (handles malformed data)
- Non-blocking command execution via pipe interface
- Safe string handling with no buffer overflows

**Key Functions**:
- `verify_adb()`: Check if ADB is accessible and working
- `list_devices()`: Enumerate connected devices and their states
- `shell_command()`: Execute arbitrary shell commands on device
- `get_property()`: Retrieve Android system properties safely
- `fastboot_command()`: Execute fastboot commands

**Implementation Notes**:
- All shell commands are properly quoted and escaped
- Output is parsed line-by-line to handle streams of any size
- Commands return std::optional to handle errors gracefully
- No external dependencies (uses POSIX popen/pclose)

#### 2. Device Inspector (`device_inspector.cpp/.h`)

**Purpose**: Extract comprehensive hardware and software information from Android devices.

**Information Extracted**:
- Hardware: CPU ABI, architecture, core count, RAM, storage
- Software: Android version, API level, build info, kernel
- Device: Manufacturer, model, codename, build fingerprint

**Key Functions**:
- `inspect()`: Full device analysis returning DeviceInfo struct
- Individual getters for specific information

**Implementation Notes**:
- Defensive parsing of /proc/cpuinfo and /proc/meminfo
- Storage parsing from `df` command output
- CPU core counting from processor entries in cpuinfo
- RAM extraction from MemTotal in meminfo
- Architecture inference from CPU ABI string

#### 3. Root Analyzer (`root_analyzer.cpp/.h`)

**Purpose**: Non-intrusive root detection via standard system markers.

**Detection Methods**:
- Check for `su` binary in standard locations
- Magisk detection via /sbin/.magisk directory
- Magisk version retrieval from `magisk --version`
- SuperSU detection via package manager
- Generic root detection via su locations

**Key Functions**:
- `analyze()`: Comprehensive root analysis
- `check_su_locations()`: Find su binary
- `check_magisk()`: Detect Magisk
- `check_supersu()`: Detect SuperSU

**Implementation Notes**:
- Non-destructive: only reads, never modifies
- Uses `which` and `test` commands for safe checks
- Prioritizes Magisk detection as most common method
- Returns RootStatus and RootMethod enums for clarity

#### 4. Bootloader Analyzer (`bootloader_analyzer.cpp/.h`)

**Purpose**: Detect bootloader lock status with clear warnings.

**Detection Methods**:
- fastboot getvar unlocked (requires bootloader mode)
- Searches for fastboot executable in standard locations

**Key Functions**:
- `analyze()`: Check bootloader status
- `get_status_via_fastboot()`: Direct fastboot query
- `get_data_loss_warning()`: Display critical warning

**Implementation Notes**:
- Locates fastboot executable before attempting checks
- Requires manual user action to reboot to bootloader
- Shows explicit warnings about data loss
- Does NOT automatically unlock or modify bootloader

#### 5. ROM Compatibility (`rom_compatibility.cpp/.h`)

**Purpose**: Offline LineageOS device compatibility lookup.

**Data Source**: Local JSON database (lineage_devices.json)

**Structure**:
```json
{
  "devices": [
    {
      "codename": "hammerhead",
      "is_supported": true,
      "latest_lineage_version": "19.1",
      "maintainer": "LineageOS Community",
      "download_url": "https://...",
      "all_versions": ["19.1", "19", "18.1", "18"]
    }
  ]
}
```

**Key Functions**:
- `load_database()`: Load JSON database file
- `check_lineage_os()`: Query compatibility
- `get_supported_devices()`: List all supported devices
- `format_rom_info()`: Pretty-print ROM information

**Implementation Notes**:
- Uses nlohmann/json library for robust JSON parsing
- Defensive error handling for malformed JSON
- Database is static (offline, no network calls)
- Extensible for other ROM projects

#### 6. Configuration Manager (`config_manager.cpp/.h`)

**Purpose**: Local persistent configuration storage.

**Configuration File**: `~/.config/lincheckroot/config.json`

**Stored Settings**:
- `adb_path`: Custom path to ADB executable
- `last_device`: Last selected device serial
- `check_updates`: Enable/disable update checks (future use)

**Key Functions**:
- `load()`: Load configuration from file
- `save()`: Persist configuration
- `get()/set()`: Access configuration values
- `verify_adb_path()`: Static method to validate ADB paths

**Implementation Notes**:
- Automatically creates config directory if needed
- Safe JSON serialization with error handling
- Loads defaults if file doesn't exist
- Creates ~/.config/lincheckroot directory with safe permissions

#### 7. GUI Layer (`gui_main.cpp/.h`)

**Purpose**: GTK4 graphical interface for device analysis.

**User Interface**:
- Single window with tabbed layout
- Control panel: ADB path input, device scanner, device selector
- Four tabs: Device Info, Root & Bootloader, Bootloader, ROM Compatibility
- Status bar for real-time feedback
- All text fields are read-only (display-only)

**Key Callbacks**:
- `on_scan_devices()`: Trigger ADB device enumeration
- `on_device_selected()`: Switch active device
- `on_refresh_info()`: Analyze selected device
- `on_adb_path_changed()`: Update ADB configuration

**Implementation Notes**:
- Uses GTK4 ComboBoxText (deprecated but still functional)
- All UI operations in main thread (non-blocking ADB calls)
- Status bar provides user feedback
- Text buffers are read-only to prevent accidental modification

## Code Organization

```
include/
  ├── adb_abstraction.h      # ADB communication interface
  ├── device_inspector.h     # Hardware info extraction
  ├── root_analyzer.h        # Root detection
  ├── bootloader_analyzer.h  # Bootloader status
  ├── rom_compatibility.h    # LineageOS lookup
  ├── config_manager.h       # Configuration storage
  └── gui_main.h             # GTK4 interface

src/
  ├── main.cpp               # Entry point
  ├── adb_abstraction.cpp    # ADB implementation
  ├── device_inspector.cpp   # Hardware inspection
  ├── root_analyzer.cpp      # Root detection logic
  ├── bootloader_analyzer.cpp # Bootloader checks
  ├── rom_compatibility.cpp  # ROM database
  ├── config_manager.cpp     # Config I/O
  └── gui_main.cpp           # GUI implementation

data/
  └── lineage_devices.json   # LineageOS device database
```

## Build System

**CMake Configuration** (`CMakeLists.txt`):
- C++17 standard with strict warnings enabled
- PkgConfig for dependency detection
- GTK4 and nlohmann_json dependencies
- Single executable target `lincheckroot`

**Build Process**:
```bash
mkdir build && cd build
cmake ..
cmake --build .
```

## Security Considerations

### No Automatic Operations
- Never installs ROMs or roots devices
- Never modifies bootloader without explicit user action
- Never executes privileged commands without warning

### No Network Operations
- All data is local
- ROM database is bundled offline
- No telemetry or analytics

### Safe Command Execution
- All shell commands are properly quoted
- Output parsing is defensive against malformed data
- No buffer overflows (uses std::string)
- No privilege escalation

### Clear User Communication
- Explicit warnings for destructive operations
- Device states clearly displayed
- Error messages are informative

## Error Handling Strategy

### Defensive Parsing
All external input (ADB output, /proc files, JSON) is validated:
- Invalid JSON treated as missing database
- Malformed /proc files handled gracefully
- Unexpected ADB output doesn't crash application

### Optional Returns
- Functions that may fail return `std::optional<T>`
- Callers must explicitly handle errors
- GUI displays "Error" messages for failed operations

### Resource Cleanup
- RAII principles used throughout
- Proper pclose() calls after popen()
- Automatic memory management with new/delete
- No resource leaks on error paths

## Performance Characteristics

### Time Complexity
- Device scanning: O(n) where n = connected devices
- Property retrieval: O(1) per property (ADB query)
- Database lookup: O(1) hashmap access
- JSON parsing: O(n) where n = JSON file size

### Space Complexity
- Minimal memory footprint (~10MB for GUI + data)
- Database: ~50KB for LineageOS devices
- Config: <1KB per user configuration

### Network Usage
- **Zero** network calls by default
- All operations are local via USB/ADB

## Testing Recommendations

### Unit Test Candidates
- ADB output parsing (defensive parsing)
- Property extraction from device info
- Root status classification
- ROM database lookups
- Configuration load/save

### Integration Test Candidates
- Full device analysis workflow
- Multiple devices connected
- Invalid ADB paths
- Missing ROM database
- Malformed JSON in config

### Manual Testing
- Connect real Android device
- Test with unauthorized device
- Test with offline device
- Test bootloader status detection
- Verify configuration persistence

## Future Extensibility

### Possible Enhancements
- Support for other ROM projects (not just LineageOS)
- Magisk module installer UI
- Bootloader unlock procedures
- Recovery image validation
- OTA update checking
- Device-specific resource links

### Proposed Architecture Changes
- Separate backend server from GUI (future web interface)
- Async device operations (current: mostly synchronous)
- Plugin system for ROM providers
- Database updates from local files

## Code Quality

### Metrics
- ~2500 lines of C++ code
- ~400 lines of CMake configuration
- ~150 lines of JSON data
- No external runtime dependencies (except GTK4, adb, fastboot)

### Style
- Consistent indentation (4 spaces)
- Clear variable and function names
- Technical comments (not for beginners)
- Defensive programming throughout

### Auditability
- All code paths visible (no external calls)
- No obfuscation or minification
- Clear error messages for debugging
- Transparent operation
