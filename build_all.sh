#!/bin/bash

# Master build script for all major Linux distributions, Windows cross-compilation, and universal packages

set -e

echo "Building Linux Speed Meter packages for all major distributions..."
echo "=================================================================="
echo "Supported distributions and package types:"
echo "  • Linux native binary"
echo "  • Windows cross-compilation (.exe)"
echo "  • Debian/Ubuntu (.deb)"
echo "  • Fedora/RHEL/CentOS (.rpm)"
echo "  • SUSE/openSUSE (.rpm)"
echo "  • Arch Linux (.pkg.tar.zst)"
echo "  • Gentoo (.ebuild)"
echo "  • Alpine Linux (.apk)"
echo "  • Snap package (.snap)"
echo "  • Flatpak package (.flatpak)"
echo "  • AppImage (.AppImage)"
echo "  • Portable bundle (.run)"
echo "=================================================================="
echo

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

# Build SUSE/openSUSE package
echo "Building SUSE/openSUSE package..."
if command_exists rpmbuild; then
    # Check if SUSE-specific build script exists
    if [ -f "packaging/build_suse.sh" ]; then
        chmod +x packaging/build_suse.sh
        if ./packaging/build_suse.sh 2>&1 | tail -20; then
            echo "✓ SUSE/openSUSE package built"
            ((BUILD_SUCCESS++))
        else
            echo "⚠ SUSE/openSUSE package build failed"
            ((BUILD_FAILED++))
        fi
    else
        echo "⚠ SUSE build script not found, skipping SUSE package"
        ((BUILD_FAILED++))
    fi
else
    echo "⚠ rpmbuild not found, skipping SUSE package"
    ((BUILD_FAILED++))
fi

echo

# Build Gentoo package
echo "Building Gentoo ebuild..."
if command_exists ebuild; then
    # Check if Gentoo build script exists
    if [ -f "packaging/build_gentoo.sh" ]; then
        chmod +x packaging/build_gentoo.sh
        if ./packaging/build_gentoo.sh 2>&1 | tail -20; then
            echo "✓ Gentoo ebuild built"
            ((BUILD_SUCCESS++))
        else
            echo "⚠ Gentoo ebuild build failed"
            ((BUILD_FAILED++))
        fi
    else
        echo "⚠ Gentoo build script not found, skipping Gentoo package"
        ((BUILD_FAILED++))
    fi
else
    echo "⚠ ebuild not found, skipping Gentoo package"
    echo "   Install ebuild: sudo emerge -av dev-util/ebuild"
    ((BUILD_FAILED++))
fi

echo

# Build Alpine Linux package
echo "Building Alpine Linux package..."
if command_exists abuild; then
    # Check if Alpine build script exists
    if [ -f "packaging/build_alpine.sh" ]; then
        chmod +x packaging/build_alpine.sh
        if ./packaging/build_alpine.sh 2>&1 | tail -20; then
            echo "✓ Alpine Linux package built"
            ((BUILD_SUCCESS++))
        else
            echo "⚠ Alpine Linux package build failed"
            ((BUILD_FAILED++))
        fi
    else
        echo "⚠ Alpine build script not found, skipping Alpine package"
        ((BUILD_FAILED++))
    fi
else
    echo "⚠ abuild not found, skipping Alpine package"
    echo "   Install abuild: sudo apk add alpine-sdk"
    ((BUILD_FAILED++))
fi

echo

# Build Snap package
echo "Building Snap package..."
if command_exists snapcraft; then
    # Check if Snap build script exists
    if [ -f "packaging/build_snap.sh" ]; then
        chmod +x packaging/build_snap.sh
        if ./packaging/build_snap.sh 2>&1 | tail -20; then
            echo "✓ Snap package built"
            ((BUILD_SUCCESS++))
        else
            echo "⚠ Snap package build failed"
            ((BUILD_FAILED++))
        fi
    else
        echo "⚠ Snap build script not found, skipping Snap package"
        ((BUILD_FAILED++))
    fi
else
    echo "⚠ snapcraft not found, skipping Snap package"
    echo "   Install snapcraft: sudo snap install snapcraft --classic"
    ((BUILD_FAILED++))
fi

echo

# Build Flatpak package
echo "Building Flatpak package..."
if command_exists flatpak-builder; then
    # Check if Flatpak build script exists
    if [ -f "packaging/build_flatpak.sh" ]; then
        chmod +x packaging/build_flatpak.sh
        if ./packaging/build_flatpak.sh 2>&1 | tail -20; then
            echo "✓ Flatpak package built"
            ((BUILD_SUCCESS++))
        else
            echo "⚠ Flatpak package build failed"
            ((BUILD_FAILED++))
        fi
    else
        echo "⚠ Flatpak build script not found, skipping Flatpak package"
        ((BUILD_FAILED++))
    fi
else
    echo "⚠ flatpak-builder not found, skipping Flatpak package"
    echo "   Install flatpak-builder: sudo apt install flatpak-builder"
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

# Check and display created packages
if [ -d "dist" ] && [ "$(ls -A dist)" ]; then
    echo "📦 Generated packages in dist/ directory:"
    ls -la dist/ | grep -E '\.(deb|rpm|pkg\.tar\.zst|AppImage|tar\.gz|run|exe|ebuild|apk|snap|flatpak)$' | sed 's/.*dist\//  /'
    echo
fi

echo "To install on your system:"
echo "  Debian/Ubuntu: sudo dpkg -i dist/linux-speed-meter_*.deb"
echo "  Fedora/RHEL/CentOS: sudo rpm -i dist/linux-speed-meter-*.rpm"
echo "  SUSE/openSUSE: sudo rpm -i dist/linux-speed-meter-*.rpm"
echo "  Arch Linux: sudo pacman -U dist/linux-speed-meter-*.pkg.tar.zst"
echo "  Gentoo: sudo ebuild dist/linux-speed-meter-*.ebuild merge"
echo "  Alpine Linux: sudo apk add --allow-untrusted dist/linux-speed-meter-*.apk"
echo "  Snap: sudo snap install dist/linux-speed-meter_*.snap --dangerous"
echo "  Flatpak: flatpak install dist/linux-speed-meter-*.flatpak"
echo "  AppImage: chmod +x dist/linux-speed-meter-*.AppImage && ./dist/linux-speed-meter-*.AppImage"
echo "  Portable Bundle: chmod +x dist/linux-speed-meter-*.run && ./dist/linux-speed-meter-*.run"
echo "  Windows: Copy dist/LinuxSpeedMeter.exe to Windows and run directly"