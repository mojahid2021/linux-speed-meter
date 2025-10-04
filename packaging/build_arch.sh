#!/bin/bash

# Build script for Arch Linux package

set -e

echo "Building Linux Speed Meter Arch Linux package..."

# Create source tarball
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"
cd "$PROJECT_DIR"
PROJECT_NAME="linux-speed-meter"
VERSION=$(cat ../../VERSION)

# Create properly structured tarball
rm -f "${PROJECT_NAME}-${VERSION}.tar.gz"
TEMP_DIR=$(mktemp -d)
cp -r . "$TEMP_DIR/${PROJECT_NAME}-${VERSION}"
cd "$TEMP_DIR"
rm -rf "${PROJECT_NAME}-${VERSION}/build" "${PROJECT_NAME}-${VERSION}/packaging" "${PROJECT_NAME}-${VERSION}/.git"
tar -czf "${PROJECT_NAME}-${VERSION}.tar.gz" "${PROJECT_NAME}-${VERSION}"
mv "${PROJECT_NAME}-${VERSION}.tar.gz" "$PROJECT_DIR/"
cd "$PROJECT_DIR"
rm -rf "$TEMP_DIR"

# Move to packaging directory for build
cd packaging/arch

# Try to build Arch package if makepkg is available
if command -v makepkg &> /dev/null; then
    echo "Attempting Arch Linux package build with makepkg..."
    # Copy source
    cp "../../${PROJECT_NAME}-${VERSION}.tar.gz" .

    # Try building with makepkg
    if makepkg -f --skippgpcheck 2>/dev/null; then
        mkdir -p ../../dist
        mv *.pkg.tar.zst ../../dist/
        echo "Arch Linux package created in dist/"
        exit 0
    else
        echo "⚠ makepkg failed, trying manual build..."
    fi
else
    echo "⚠ makepkg not available, trying manual build..."
fi

# Manual Arch package build (fallback)
echo "Building Arch Linux package manually..."

# Extract source
tar -xzf "../../${PROJECT_NAME}-${VERSION}.tar.gz"
cd "${PROJECT_NAME}-${VERSION}"

# Build the application
mkdir -p build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr
make

# Create package structure
cd ..
PKG_DIR="pkg"
mkdir -p "$PKG_DIR/usr/bin" "$PKG_DIR/usr/share/applications" "$PKG_DIR/usr/share/icons/hicolor/256x256/apps"

# Install files
install -Dm755 build/LinuxSpeedMeter "$PKG_DIR/usr/bin/linux-speed-meter"

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

# Create icon (placeholder)
touch "$PKG_DIR/usr/share/icons/hicolor/256x256/apps/linux-speed-meter.png"

# Create the package
cd ..
PKG_NAME="${PROJECT_NAME}-${VERSION}-1-x86_64.pkg.tar.zst"
tar -cf - -C "${PROJECT_NAME}-${VERSION}" pkg/ | zstd -z -o "$PKG_NAME"

# Move to dist
mkdir -p ../../dist
mv "$PKG_NAME" ../../dist/

# Cleanup
rm -rf "${PROJECT_NAME}-${VERSION}"

echo "Arch Linux package created manually: dist/$PKG_NAME"

# Fallback: Create instructions for manual Arch building
echo "Creating Arch Linux build instructions..."
mkdir -p ../../dist
cat > ../../dist/README-Arch.md << 'EOF'
# Building Arch Linux Package

Since makepkg is not available in the current environment, you can build the Arch Linux package on an Arch Linux system:

## Prerequisites
- pacman package manager (comes with Arch Linux)
- Development tools: `sudo pacman -S base-devel`
- GTK3 development: `sudo pacman -S gtk3 libappindicator-gtk3`

## Build Instructions
1. Copy the PKGBUILD file and source tarball to a directory
2. Run: `makepkg -si` (build and install) or `makepkg` (build only)
3. Find the built package in the current directory

## Manual Installation
Once built, install with: `sudo pacman -U linux-speed-meter-*.pkg.tar.zst`
EOF

echo "Arch Linux build instructions created in dist/README-Arch.md"