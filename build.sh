#!/bin/bash
# Quick build script for development

set -e

echo "Building Linux Speed Meter..."

# Parse arguments
BUILD_TYPE="${BUILD_TYPE:-Release}"
CLEAN_BUILD=false

while [[ $# -gt 0 ]]; do
    case $1 in
        clean)
            CLEAN_BUILD=true
            shift
            ;;
        debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        release)
            BUILD_TYPE="Release"
            shift
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 [clean] [debug|release]"
            exit 1
            ;;
    esac
done

# Clean and rebuild if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

mkdir -p build
cd build

# Configure
echo "Configuring build type: $BUILD_TYPE"
cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE

# Build
echo "Building with $(nproc) parallel jobs..."
make -j$(nproc)

echo "Build completed! Run ./build/LinuxSpeedMeter to test."