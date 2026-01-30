# LinCheckROOT - Android Device Analyzer

A native Linux GUI application for comprehensive analysis of Android devices connected via USB.

## Features

- **Device Detection**: Enumerate connected Android devices via ADB
- **Device Information**: Extract hardware specs, Android version, kernel info
- **Root Analysis**: Non-intrusive root detection (Magisk, SuperSU)
- **Bootloader Status**: Check bootloader lock state (requires fastboot)
- **ROM Compatibility**: Offline LineageOS compatibility check
- **Configurable ADB Path**: Use custom ADB installation
- **No Telemetry**: Completely local, no internet required

## Requirements

### Build Dependencies
- CMake >= 3.22
- GCC/Clang with C++17 support
- GTK4 development files
- nlohmann/json development files

### Runtime Dependencies
- GTK4 runtime
- ADB (Android Debug Bridge) - can be provided by user
- fastboot (optional, for bootloader checks)

### System Requirements
- Linux x86-64
- USB connection to Android device
- USB debugging enabled on device

## Installation

### Install Build Dependencies

**Debian/Ubuntu:**
```bash
sudo apt-get update
sudo apt-get install -y \
    cmake \
    g++ \
    libgtk-4-dev \
    nlohmann-json3-dev
```

**Fedora/RHEL:**
```bash
sudo dnf install -y \
    cmake \
    gcc-c++ \
    gtk4-devel \
    nlohmann_json-devel
```

**Arch Linux:**
```bash
sudo pacman -S \
    cmake \
    gcc \
    gtk4 \
    nlohmann-json
```

## Build

```bash
cd /home/mihai/Desktop/LinCheckROOT
mkdir -p build
cd build
cmake ..
cmake --build .
```

## Run

```bash
./build/lincheckroot
```

Or directly from project root:
```bash
./build/lincheckroot
```

## Configuration

Configuration file: `~/.config/lincheckroot/config.json`

Example:
```json
{
    "adb_path": "/path/to/custom/adb",
    "last_device": "device_serial",
    "check_updates": false
}
```

## Usage

1. **Scan Devices**: Click "Scan Devices" button to enumerate connected devices
2. **Select Device**: Choose a device from the dropdown
3. **Configure ADB** (if needed): Enter custom ADB path in the "ADB Path" field
4. **Refresh**: Click "Refresh Information" to analyze the selected device
5. **View Results**: Check tabs for device info, root status, bootloader status, and ROM compatibility

## How to Enable USB Debugging on Android

1. Go to **Settings** > **About Phone**
2. Tap **Build Number** 7 times to unlock Developer Options
3. Go to **Settings** > **Developer Options**
4. Enable **USB Debugging**
5. Connect via USB and accept the RSA fingerprint prompt on the device

## Architecture

The application is modular:

- **adb_abstraction**: ADB communication layer with defensive parsing
- **device_inspector**: Hardware and software information extraction
- **root_analyzer**: Non-intrusive root detection
- **bootloader_analyzer**: Bootloader status detection
- **rom_compatibility**: Offline LineageOS compatibility database
- **config_manager**: Local configuration management
- **gui_main**: GTK4 graphical interface

## Security & Privacy

- **No telemetry**: All operations are local
- **No network**: Offline operation (except ROM database included locally)
- **No automatic installation**: Only analyzes, never modifies device
- **Clear warnings**: Explicit warnings for destructive operations
- **Open source**: Code is readable and auditable

## Limitations

- Bootloader status requires device in bootloader mode (manual reboot needed)
- Some properties may not be available on all devices
- ROM compatibility database is static (LineageOS devices)

## Technical Notes

- Parsing of ADB output is defensive against malformed data
- All shell commands use proper escaping
- No elevation to root required (respects user permissions)
- GTK4 provides non-blocking async operations

## Project Structure

```
LinCheckROOT/
├── CMakeLists.txt          # Build configuration
├── include/                # Header files
│   ├── adb_abstraction.h
│   ├── device_inspector.h
│   ├── root_analyzer.h
│   ├── bootloader_analyzer.h
│   ├── rom_compatibility.h
│   ├── config_manager.h
│   └── gui_main.h
├── src/                    # Implementation files
│   ├── main.cpp
│   ├── adb_abstraction.cpp
│   ├── device_inspector.cpp
│   ├── root_analyzer.cpp
│   ├── bootloader_analyzer.cpp
│   ├── rom_compatibility.cpp
│   ├── config_manager.cpp
│   └── gui_main.cpp
├── data/                   # Data files
│   └── lineage_devices.json
└── build/                  # Build directory
```

## License

Open source. Audit-friendly code without obfuscation.

## Troubleshooting

### ADB not found
- Specify custom ADB path in the GUI
- Or install Android SDK Platform Tools

### Device shows "unauthorized"
- Accept RSA fingerprint prompt on device
- Check Developer Options are enabled
- Restart ADB: `adb kill-server && adb devices`

### No devices detected
- Check USB cable (try different cable)
- Enable USB Debugging in Developer Options
- Check device appears in system with `lsusb`

### Boot lock status unknown
- Connect device and manually reboot to bootloader
- Some devices don't expose bootloader status via ADB
