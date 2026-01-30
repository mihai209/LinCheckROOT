#!/bin/bash
# Installation script for LinCheckROOT

set -e

PROJECT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_DIR/build"

# Check if binary exists
if [ ! -f "$BUILD_DIR/lincheckroot" ]; then
    echo "[!] Binary not found. Run ./build.sh first"
    exit 1
fi

echo "=========================================="
echo "LinCheckROOT Installation"
echo "=========================================="
echo ""

# Ask for installation prefix
PREFIX="${1:-/usr/local}"
echo "Installation prefix: $PREFIX"
echo ""

# Create installation directories
echo "[*] Creating directories..."
sudo mkdir -p "$PREFIX/bin"
sudo mkdir -p "$PREFIX/share/lincheckroot"
sudo mkdir -p "$PREFIX/share/applications"
sudo mkdir -p "$PREFIX/share/icons/hicolor/scalable/apps"

# Install executable
echo "[*] Installing executable..."
sudo cp "$BUILD_DIR/lincheckroot" "$PREFIX/bin/"
sudo chmod 755 "$PREFIX/bin/lincheckroot"

# Install data files
echo "[*] Installing data files..."
sudo cp "$PROJECT_DIR/data/lineage_devices.json" "$PREFIX/share/lincheckroot/"

# Create desktop entry
echo "[*] Creating desktop entry..."
sudo tee "$PREFIX/share/applications/lincheckroot.desktop" > /dev/null <<EOF
[Desktop Entry]
Type=Application
Name=LinCheckROOT
Comment=Android Device Analyzer
Exec=$PREFIX/bin/lincheckroot
Icon=lincheckroot
Categories=Utility;System;
Terminal=false
StartupNotify=true
Keywords=android;adb;root;device;analyzer;
EOF

# Create simple icon (text-based)
echo "[*] Creating icon..."
sudo tee "$PREFIX/share/icons/hicolor/scalable/apps/lincheckroot.svg" > /dev/null <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<svg xmlns="http://www.w3.org/2000/svg" width="128" height="128" viewBox="0 0 128 128">
  <rect width="128" height="128" fill="#2196F3" rx="24"/>
  <text x="64" y="70" font-size="48" font-weight="bold" fill="white" text-anchor="middle" font-family="monospace">LCR</text>
  <rect x="20" y="20" width="88" height="88" fill="none" stroke="white" stroke-width="3" rx="8"/>
</svg>
EOF

echo ""
echo "=========================================="
echo "[✓] Installation complete!"
echo "=========================================="
echo ""
echo "Run with: lincheckroot"
echo "Or: $PREFIX/bin/lincheckroot"
echo ""
echo "To uninstall:"
echo "  sudo rm $PREFIX/bin/lincheckroot"
echo "  sudo rm -rf $PREFIX/share/lincheckroot"
echo "  sudo rm $PREFIX/share/applications/lincheckroot.desktop"
echo "  sudo rm $PREFIX/share/icons/hicolor/scalable/apps/lincheckroot.svg"
echo ""
