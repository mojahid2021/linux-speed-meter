#!/bin/bash

# Master build script for all Linux distributions and Windows cross-compilation

set -e

echo "Building Linux Speed Meter packages for all distributions..."
echo "========================================================"

PROJECT_ROOT="$(pwd)"

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Build Windows cross-compilation first
echo "Building Windows cross-compilation..."
if command_exists x86_64-w64-mingw32-gcc; then
    chmod +x build_windows_cross.sh
    if ./build_windows_cross.sh; then
        echo "✓ Windows cross-compilation successful (LinuxSpeedMeter.exe created)"
    else
        echo "⚠ Windows cross-compilation failed"
    fi
else
    echo "⚠ MinGW-w64 not found, skipping Windows cross-compilation"
    echo "   Install MinGW-w64: sudo apt install mingw-w64 g++-mingw-w64-x86-64-win32"
fi

echo

# Build Debian package
echo "Building Debian (.deb) package..."
if command_exists dpkg-deb; then
    chmod +x packaging/build_debian.sh
    ./packaging/build_debian.sh
    echo "✓ Debian package built"
else
    echo "⚠ dpkg-deb not found, skipping Debian package"
fi

echo

# Build RPM package
echo "Building RPM package..."
if command_exists rpmbuild; then
    chmod +x packaging/build_rpm.sh
    ./packaging/build_rpm.sh
    echo "✓ RPM package built"
else
    echo "⚠ rpmbuild not found, skipping RPM package"
    echo "   Install rpmbuild: sudo dnf install rpm-build  (Fedora/RHEL)"
    echo "                    sudo apt install rpm          (Ubuntu/Debian)"
fi

echo

# Build Arch package
echo "Building Arch Linux package..."
if command_exists makepkg; then
    chmod +x packaging/build_arch.sh
    ./packaging/build_arch.sh
    echo "✓ Arch Linux package built"
else
    echo "⚠ makepkg not found, skipping Arch package"
    echo "   Install makepkg: sudo pacman -S pacman"
fi

echo

# Build AppImage
echo "Building AppImage..."
chmod +x packaging/build_appimage.sh
if ./packaging/build_appimage.sh; then
    echo "✓ AppImage or portable bundle built"
else
    echo "⚠ AppImage/portable bundle build failed"
fi

echo
echo "========================================================"
echo "Package building complete!"
echo
echo "Generated packages in dist/ directory:"
echo

# Check and display created packages
if [ -d "dist" ] && [ "$(ls -A dist)" ]; then
    echo "📦 Generated packages in dist/ directory:"
    ls -la dist/ | grep -E '\.(deb|rpm|pkg\.tar\.zst|AppImage|tar\.gz|run|exe)$' | sed 's/.*dist\//  /'
    echo
fi

echo "To install on your system:"
echo "  Debian/Ubuntu: sudo dpkg -i dist/linux-speed-meter_*.deb"
echo "  Fedora/RHEL: sudo rpm -i dist/linux-speed-meter-*.rpm"
echo "  Arch Linux: sudo pacman -U dist/linux-speed-meter-*.pkg.tar.zst"
echo "  Portable Bundle: chmod +x dist/linux-speed-meter-*.run && ./dist/linux-speed-meter-*.run"
echo "  Windows: Copy dist/LinuxSpeedMeter.exe to Windows and run directly"