# LinCheckROOT - User Guide

## Quick Start

### 1. Ensure Prerequisites
- Linux x86-64 system
- USB cable and Android device
- Developer Options enabled on device
- USB Debugging enabled on device

### 2. Build the Application

```bash
cd /home/mihai/Desktop/LinCheckROOT
./build.sh
```

### 3. Run the Application

```bash
./build/lincheckroot
```

## First-Time Setup

### Enable USB Debugging on Android Device

1. **Unlock Developer Options**:
   - Open Settings
   - Scroll to "About Phone" (or "About Device")
   - Find "Build Number"
   - Tap "Build Number" 7 times
   - A message appears: "You are now a developer"

2. **Enable USB Debugging**:
   - Go back to Settings
   - Look for "Developer Options" (usually under System settings)
   - Scroll down and find "USB Debugging"
   - Enable the toggle

3. **Accept RSA Fingerprint**:
   - Connect device via USB
   - A dialog appears: "Allow USB debugging?"
   - Tap "Allow" to accept the connection
   - The device will remember this computer

### Configure Custom ADB Path (if needed)

If ADB is not in standard locations:

1. Click in the "ADB Path" field
2. Enter the full path to your ADB binary
3. Example: `/home/user/android-sdk/platform-tools/adb`
4. The application will verify the path automatically
5. Configuration is saved to `~/.config/lincheckroot/config.json`

## Using LinCheckROOT

### Main Workflow

1. **Connect Device**: Use USB cable to connect Android device

2. **Scan Devices**: 
   - Click "Scan Devices" button
   - The application searches for connected Android devices
   - Devices appear in the dropdown list with their states:
     - `device` - Connected and ready
     - `unauthorized` - Accept RSA fingerprint on device
     - `offline` - Device detected but not responding

3. **Select Device**:
   - Choose device from dropdown if multiple connected
   - The application confirms selection in status bar

4. **Analyze Device**:
   - Click "Refresh Information" button
   - Application gathers comprehensive device data
   - Results appear in four tabs:
     - **Device Info**: Hardware and software specifications
     - **Root & Bootloader**: Root status and detection method
     - **Bootloader**: Bootloader lock status (detailed information)
     - **ROM Compatibility**: LineageOS support status

### Understanding the Results

#### Device Info Tab
Shows:
- **Manufacturer & Model**: Who made the device and model name
- **Codename**: Internal device identifier (important for ROM flashing)
- **Android Version**: System version number
- **API Level**: Android SDK version number
- **CPU ABI**: Processor instruction set (arm64-v8a, armeabi-v7a, etc.)
- **CPU Cores**: Number of processor cores
- **RAM**: Total memory in MB
- **Storage**: Total and free space on /data partition
- **Build Info**: Build date, type, and fingerprint

#### Root & Bootloader Tab
Shows:
- **Root Status**: "Rooted" or "Not Rooted"
- **Root Method**: How root was obtained:
  - **Magisk**: Modern root manager (recommended)
  - **SuperSU**: Legacy root manager
  - **Unknown Method**: Root found but method unclear
  - **Not Rooted**: No root detected

If Magisk is installed:
- Shows Magisk version number

#### Bootloader Tab
Shows:
- **Lock Status**: Current bootloader lock state
- **Fastboot Available**: Whether fastboot tool can be used
- **Data Loss Warning**: Critical warning about bootloader unlocking

⚠️ **Important**: Unlocking bootloader erases all device data!

#### ROM Compatibility Tab
Shows:
- **Support Status**: Is LineageOS officially supported?
- **Latest Version**: Newest LineageOS version available
- **Maintainer**: Developer who maintains this device
- **Download URL**: Where to get the ROM
- **Available Versions**: All LineageOS versions supported

## Troubleshooting

### "ADB not found"
**Solution**:
1. Install Android SDK Platform Tools from Google
2. Or specify custom ADB path in the application
3. Click "Scan Devices" to verify ADB is working

### "No devices found"
**Check**:
1. Device is connected with USB cable
2. Device shows up in `lsusb` output:
   ```bash
   lsusb
   ```
3. USB Debugging is enabled in Developer Options
4. Accept the RSA fingerprint prompt on device
5. Try: `adb kill-server && adb devices`

### "Device shows unauthorized"
**Solution**:
1. Look for "Allow USB Debugging?" prompt on device screen
2. Tap "Allow" to accept the connection
3. Click "Scan Devices" again

### "Device offline"
**Solution**:
1. Disconnect and reconnect USB cable
2. Try different USB port
3. Restart device in developer mode
4. Restart ADB: `adb kill-server`

### "ROM not in database"
**Explanation**:
- Device may not be officially supported by LineageOS
- Look up device codename at https://wiki.lineageos.org
- Community ports may exist but aren't in database

### Application won't start
**Check dependencies**:
```bash
gtk4-launch --version
adb version
```

If missing:
```bash
sudo apt-get install libgtk-4-1 android-tools-adb
```

## Advanced Usage

### Using Custom ADB Installation
If you have a custom Android SDK:

1. Find your ADB path:
   ```bash
   find /path/to/android-sdk -name adb
   ```

2. In LinCheckROOT:
   - Enter path in "ADB Path" field
   - Application will verify it works
   - Configuration is persistent

### Exporting Device Information
1. Select all text in each tab (Ctrl+A)
2. Copy to clipboard (Ctrl+C)
3. Paste into text file or document

### Checking Device is Safe
LinCheckROOT can help identify:
- If device is rooted (potential security risk)
- If bootloader is unlocked (security consideration)
- If running custom ROM (may have security implications)

## Security Notes

### What LinCheckROOT Does NOT Do
- ✅ Never installs ROMs
- ✅ Never roots devices
- ✅ Never modifies system files
- ✅ Never sends data to internet
- ✅ Never stores analytics or telemetry

### What LinCheckROOT Only Reads
- ✅ Device properties (getprop)
- ✅ File existence checks (test -f)
- ✅ System information (/proc files)
- ✅ Bootloader lock status (read-only query)

### Safe Practices
1. Accept connection only when you initiate
2. Verify device fingerprint on screen
3. Disconnect device when not in use
4. Keep ADB path correctly configured
5. Use official Android SDK tools

## Configuration Files

### Main Configuration
- **Location**: `~/.config/lincheckroot/config.json`
- **Format**: JSON
- **Example**:
```json
{
    "adb_path": "/usr/bin/adb",
    "last_device": "emulator-5554",
    "check_updates": false
}
```

### ROM Database
- **Location**: `/usr/share/lincheckroot/lineage_devices.json`
- **Fallback**: `./data/lineage_devices.json` (in build directory)
- **Format**: JSON with device list

## Frequently Asked Questions

**Q: Can I use LinCheckROOT on other Linux distributions?**
A: Yes, any x86-64 Linux with GTK4 installed. Build process is the same.

**Q: Why does bootloader status show "Unknown"?**
A: Some manufacturers don't expose bootloader status via ADB. You need to manually reboot to bootloader mode to check with fastboot.

**Q: Is it safe to unlock my bootloader?**
A: Unlocking bootloader ERASES all data. Always backup before unlocking. The application warns about this prominently.

**Q: Can I flash ROMs with LinCheckROOT?**
A: No, LinCheckROOT only analyzes devices. Use separate tools like TWRP recovery for flashing.

**Q: What's the difference between rooted and having custom ROM?**
A: **Rooted** = elevated user access (su binary). **Custom ROM** = different Android OS installed. These are independent.

**Q: How often is the LineageOS database updated?**
A: The database is built-in and static. For current information, visit https://wiki.lineageos.org

**Q: Can I use LinCheckROOT on Windows or Mac?**
A: Currently Linux only. Would require GTK4 port for other platforms.

## Getting Help

### Check Device Manually
```bash
# List connected devices
adb devices -l

# Get device properties
adb shell getprop ro.product.model
adb shell getprop ro.build.version.release

# Check for root
adb shell which su
adb shell test -d /sbin/.magisk && echo Magisk detected
```

### Enable ADB Debugging Logs
```bash
adb logcat | grep -i debug
```

### Verify Fastboot
```bash
fastboot devices
fastboot getvar unlocked
```

## Uninstallation

To remove LinCheckROOT:

```bash
# Remove built application
rm -rf /home/mihai/Desktop/LinCheckROOT/build/lincheckroot

# Remove configuration (optional)
rm -rf ~/.config/lincheckroot/

# Remove source code (if desired)
rm -rf /home/mihai/Desktop/LinCheckROOT
```

To remove dependencies (if not needed by other applications):
```bash
sudo apt-get autoremove libgtk-4-0 nlohmann-json3
```

---

**Report issues or questions** to the application author or project repository.
