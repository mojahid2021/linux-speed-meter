#!/bin/bash

# Build script for Debian package

set -e

echo "Building Linux Speed Meter Debian package..."

# Build the application
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_DIR"
mkdir -p build
cd build
cmake ..
make

# Create package structure
cd ..
PKG_DIR="packaging/debian"
mkdir -p "$PKG_DIR/usr/bin"
mkdir -p "$PKG_DIR/usr/share/applications"
mkdir -p "$PKG_DIR/usr/share/icons/hicolor/256x256/apps"

# Copy files
cp "build/LinuxSpeedMeter" "$PKG_DIR/usr/bin/linux-speed-meter"

# Create desktop file
cat > "$PKG_DIR/usr/share/applications/linux-speed-meter.desktop" << EOF
[Desktop Entry]
Type=Application
Name=Linux Speed Meter
Comment=Real-time network speed monitor
Exec=linux-speed-meter
Icon=linux-speed-meter
Categories=Utility;Network;Monitor;
StartupNotify=false
Terminal=false
EOF

# Create a simple icon (placeholder - you should replace with actual icon)
convert -size 256x256 xc:"#4A90E2" -fill white -pointsize 72 -gravity center -annotate +0+0 "LSM" "$PKG_DIR/usr/share/icons/hicolor/256x256/apps/linux-speed-meter.png" 2>/dev/null || {
    # Fallback if ImageMagick is not available
    echo "Warning: ImageMagick not available, skipping icon creation"
    touch "$PKG_DIR/usr/share/icons/hicolor/256x256/apps/linux-speed-meter.png"
}

# Set permissions
chmod 755 "$PKG_DIR/usr/bin/linux-speed-meter"
chmod 644 "$PKG_DIR/usr/share/applications/linux-speed-meter.desktop"
chmod 644 "$PKG_DIR/usr/share/icons/hicolor/256x256/apps/linux-speed-meter.png"

# Build the package
cd packaging
dpkg-deb --build debian linux-speed-meter_1.0.0_amd64.deb

# Move to dist directory
mkdir -p ../dist
mv linux-speed-meter_1.0.0_amd64.deb ../dist/

echo "Debian package created: dist/linux-speed-meter_1.0.0_amd64.deb"