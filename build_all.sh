#!/bin/bash

# Master build script for all Linux distributions and Windows cross-compilation

set -e

echo "Building Linux Speed Meter packages for all distributions..."
echo "========================================================"

PROJECT_ROOT="$(pwd)"
BUILD_SUCCESS=0
BUILD_FAILED=0

# Function to check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Create dist directory if it doesn't exist
mkdir -p dist

# Build Linux native binary first
echo "Building Linux native binary..."
if command_exists cmake && command_exists make; then
    chmod +x build.sh
    if ./build.sh; then
        echo "✓ Linux native build successful"
        ((BUILD_SUCCESS++))
    else
        echo "⚠ Linux native build failed"
        ((BUILD_FAILED++))
    fi
else
    echo "⚠ CMake or Make not found, skipping Linux native build"
    echo "   Install: sudo apt install cmake build-essential"
    ((BUILD_FAILED++))
fi

echo

# Build Windows cross-compilation
echo "Building Windows cross-compilation..."
if command_exists x86_64-w64-mingw32-gcc; then
    chmod +x build_windows_cross.sh
    if ./build_windows_cross.sh; then
        echo "✓ Windows cross-compilation successful (LinuxSpeedMeter.exe created)"
        ((BUILD_SUCCESS++))
    else
        echo "⚠ Windows cross-compilation failed"
        ((BUILD_FAILED++))
    fi
else
    echo "⚠ MinGW-w64 not found, skipping Windows cross-compilation"
    echo "   Install MinGW-w64: sudo apt install mingw-w64 g++-mingw-w64-x86-64-win32"
    ((BUILD_FAILED++))
fi

echo

# Build Debian package
echo "Building Debian (.deb) package..."
if command_exists dpkg-deb; then
    chmod +x packaging/build_debian.sh
    if ./packaging/build_debian.sh 2>&1 | tail -20; then
        echo "✓ Debian package built"
        ((BUILD_SUCCESS++))
    else
        echo "⚠ Debian package build failed"
        ((BUILD_FAILED++))
    fi
else
    echo "⚠ dpkg-deb not found, skipping Debian package"
    ((BUILD_FAILED++))
fi

echo

# Build RPM package
echo "Building RPM package..."
if command_exists rpmbuild; then
    chmod +x packaging/build_rpm.sh
    if ./packaging/build_rpm.sh 2>&1 | tail -20; then
        echo "✓ RPM package built"
        ((BUILD_SUCCESS++))
    else
        echo "⚠ RPM package build failed"
        ((BUILD_FAILED++))
    fi
else
    echo "⚠ rpmbuild not found, skipping RPM package"
    echo "   Install rpmbuild: sudo dnf install rpm-build  (Fedora/RHEL)"
    echo "                    sudo apt install rpm          (Ubuntu/Debian)"
    ((BUILD_FAILED++))
fi

echo

# Build Arch package
echo "Building Arch Linux package..."
if command_exists makepkg; then
    chmod +x packaging/build_arch.sh
    if ./packaging/build_arch.sh 2>&1 | tail -20; then
        echo "✓ Arch Linux package built"
        ((BUILD_SUCCESS++))
    else
        echo "⚠ Arch Linux package build failed"
        ((BUILD_FAILED++))
    fi
else
    echo "⚠ makepkg not found, skipping Arch package"
    echo "   Install makepkg: sudo pacman -S pacman"
    ((BUILD_FAILED++))
fi

echo

# Build AppImage
echo "Building AppImage..."
chmod +x packaging/build_appimage.sh
if ./packaging/build_appimage.sh 2>&1 | tail -20; then
    echo "✓ AppImage or portable bundle built"
    ((BUILD_SUCCESS++))
else
    echo "⚠ AppImage/portable bundle build failed"
    ((BUILD_FAILED++))
fi

echo
echo "========================================================"
echo "Package building complete!"
echo "Successful builds: $BUILD_SUCCESS"
echo "Failed builds: $BUILD_FAILED"
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