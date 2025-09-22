#!/bin/bash

# Linux Speed Meter - Cleanup Script
# Removes build artifacts, temporary files, and unnecessary files

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR" && pwd)"

echo "🧹 Linux Speed Meter Cleanup Script"
echo "=================================="
echo "Project root: $PROJECT_ROOT"
echo

# Function to safely remove files/directories
safe_remove() {
    local path="$1"
    if [ -e "$path" ]; then
        echo "Removing: $path"
        if rm -rf "$path" 2>/dev/null; then
            echo "Removed successfully"
        else
            echo "Permission denied, trying with sudo..."
            if sudo rm -rf "$path" 2>/dev/null; then
                echo "Removed with sudo"
            else
                echo "Failed to remove: $path (insufficient permissions)"
            fi
        fi
    else
        echo "Not found: $path"
    fi
}

# Function to show disk usage
show_usage() {
    echo
    echo "📊 Disk Usage Summary:"
    if command -v du >/dev/null 2>&1; then
        du -sh "$PROJECT_ROOT" 2>/dev/null || echo "Unable to calculate disk usage"
    fi
}

# Show initial disk usage
show_usage

echo
echo "🗂️  Cleaning up build artifacts..."

# Build directories
safe_remove "build"
safe_remove "build-qt"
safe_remove "build-windows"
safe_remove "packaging/appimage/AppDir"
safe_remove "packaging/fedora/BUILD"
safe_remove "packaging/fedora/BUILDROOT"
safe_remove "packaging/fedora/RPMS"
safe_remove "packaging/fedora/SRPMS"
safe_remove "packaging/fedora/SOURCES"
safe_remove "packaging/arch/linux-speed-meter-*/"
safe_remove "CMakeFiles"
safe_remove "CMakeCache.txt"
safe_remove "cmake_install.cmake"
safe_remove "Makefile"
safe_remove "*.cbp"
safe_remove ".vs"
safe_remove "x64"
safe_remove "Debug"
safe_remove "Release"

echo
echo "📦 Cleaning up packaging artifacts..."

# Distribution packages
safe_remove "dist"
safe_remove "*.deb"
safe_remove "*.rpm"
safe_remove "*.pkg.tar.zst"
safe_remove "*.AppImage"
safe_remove "*.exe"  # Windows executables
safe_remove "*.run"  # Portable bundles
safe_remove "*.run"
safe_remove "*.tar.gz"
safe_remove "*.tar.bz2"

echo
echo "🗑️  Cleaning up temporary and cache files..."

# Temporary files
find "$PROJECT_ROOT" -name "*.tmp" -type f -delete 2>/dev/null || true
find "$PROJECT_ROOT" -name "*.temp" -type f -delete 2>/dev/null || true
find "$PROJECT_ROOT" -name "*~" -type f -delete 2>/dev/null || true
find "$PROJECT_ROOT" -name "*.bak" -type f -delete 2>/dev/null || true
find "$PROJECT_ROOT" -name "*.orig" -type f -delete 2>/dev/null || true
find "$PROJECT_ROOT" -name "*.swp" -type f -delete 2>/dev/null || true
find "$PROJECT_ROOT" -name "*.swo" -type f -delete 2>/dev/null || true
find "$PROJECT_ROOT" -name ".DS_Store" -type f -delete 2>/dev/null || true
find "$PROJECT_ROOT" -name "Thumbs.db" -type f -delete 2>/dev/null || true

# CMake cache and build files
find "$PROJECT_ROOT" -name "CMakeCache.txt" -type f -delete 2>/dev/null || true
find "$PROJECT_ROOT" -name "cmake_install.cmake" -type f -delete 2>/dev/null || true
find "$PROJECT_ROOT" -name "Makefile" -type f -delete 2>/dev/null || true
find "$PROJECT_ROOT" -name "*.cbp" -type f -delete 2>/dev/null || true

# Python cache
find "$PROJECT_ROOT" -name "__pycache__" -type d -exec rm -rf {} + 2>/dev/null || true
find "$PROJECT_ROOT" -name "*.pyc" -type f -delete 2>/dev/null || true
find "$PROJECT_ROOT" -name "*.pyo" -type f -delete 2>/dev/null || true

# Node.js cache (if any)
find "$PROJECT_ROOT" -name "node_modules" -type d -exec rm -rf {} + 2>/dev/null || true
find "$PROJECT_ROOT" -name "package-lock.json" -type f -delete 2>/dev/null || true
find "$PROJECT_ROOT" -name "yarn.lock" -type f -delete 2>/dev/null || true

echo
echo "🧪 Cleaning up test artifacts..."

# Test files and directories
safe_remove "test_autostart"
safe_remove "test_autostart.cpp"
safe_remove "testing"
safe_remove "tests"
safe_remove "*.log"
safe_remove "*.out"
safe_remove "core"
safe_remove "core.*"

echo
echo "📁 Cleaning up empty directories..."

# Remove empty directories (be careful not to remove important ones)
find "$PROJECT_ROOT" -type d -empty -delete 2>/dev/null || true

echo
echo "✅ Cleanup completed successfully!"

# Show final disk usage
show_usage

echo
echo "💡 Tips:"
echo "  - Run './build_all.sh' to rebuild everything"
echo "  - Run 'make clean && make' in build directory for partial cleanup"
echo "  - Use 'git clean -fdx' for git-tracked cleanup (CAUTION: removes untracked files)"
echo
echo "🎉 Project is now clean and ready for fresh builds!"