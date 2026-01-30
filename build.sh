#!/bin/bash
# Build script for LinCheckROOT

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

echo "=========================================="
echo "LinCheckROOT Build Script"
echo "=========================================="
echo ""

# Check for required tools
echo "[*] Checking for required tools..."

if ! command -v cmake &> /dev/null; then
    echo "[!] CMake not found. Please install CMake >= 3.22"
    exit 1
fi

if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null; then
    echo "[!] C++ compiler not found. Please install GCC or Clang"
    exit 1
fi

if ! pkg-config --exists gtk4; then
    echo "[!] GTK4 development files not found"
    echo "    Install with: sudo apt-get install libgtk-4-dev"
    exit 1
fi

if ! pkg-config --exists nlohmann_json; then
    echo "[!] nlohmann_json not found"
    echo "    Install with: sudo apt-get install nlohmann-json3-dev"
    exit 1
fi

echo "[✓] All dependencies found"
echo ""

# Create build directory
echo "[*] Creating build directory..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Run CMake
echo "[*] Configuring build with CMake..."
cmake "$PROJECT_DIR"

# Build
echo "[*] Building LinCheckROOT..."
cmake --build . --config Release -j$(nproc)

# Verify executable
if [ -f "$BUILD_DIR/lincheckroot" ]; then
    echo ""
    echo "=========================================="
    echo "[✓] Build successful!"
    echo "=========================================="
    echo ""
    echo "Executable: $BUILD_DIR/lincheckroot"
    echo ""
    echo "To run the application:"
    echo "  $BUILD_DIR/lincheckroot"
    echo ""
    echo "Install system-wide (optional):"
    echo "  sudo cmake --install . --prefix /usr/local"
    echo ""
else
    echo "[!] Build failed - executable not found"
    exit 1
fi
